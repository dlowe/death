NAME := death
CODE := $(NAME).c
OBJ  := $(CODE:.c=.o)

DNAME := $(NAME)-data
DCODE := $(DNAME).c
DATA  := splash.d dead.d sprites.d

CC      := gcc
CFLAGS  := -Wall -I/usr/X11R6/include -D_BSD_SOURCE -O3
LDFLAGS := -L/usr/X11R6/lib -lX11

.PHONY: all
all: $(NAME)

CODE_SIZE := $(shell cat $(CODE) | wc -c)
RULE_SIZE := $(shell cat $(CODE) | perl -pe 's/[;{}]\s//g' | perl -pe 's/\s//g' | wc -c)

.PHONY: test
test: prog
	@echo "code size $(CODE_SIZE) / 4096"
	@test $(CODE_SIZE) -le 4096
	@echo "rule size $(RULE_SIZE) / 2048"
	@test $(RULE_SIZE) -le 2048
	@echo "testing README.markdown"
	@Markdown.pl README.markdown >/dev/null

$(DATA): $(DNAME)
	./$(DNAME)

prog.c: $(CODE)
	cp $(CODE) prog.c

include build.mk

$(NAME): $(DATA) test $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(LDFLAGS)

$(DNAME): $(DCODE) $(CODE)
	$(CC) $(CFLAGS) -std=c99 -o $@ $(DCODE) $(LDFLAGS)

.PHONY: clean
clean:
	rm -rf $(NAME) $(OBJ)
	rm -rf prog.c prog
	rm -rf $(DNAME)
	rm -rf $(DNAME).dSYM
	rm -rf $(DATA)
