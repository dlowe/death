NAME := death
CODE := $(NAME).c
OBJ  := $(CODE:.c=.o)

DNAME := $(NAME)-data
DCODE := $(DNAME).c
DATA  := 0.d 1.d 2.d

CC      := gcc
CFLAGS  := -Wall -pedantic-errors -I/usr/X11R6/include -L/usr/X11R6/lib -D_BSD_SOURCE -O3
LDFLAGS := -lX11

.PHONY: all
all: $(NAME)

CODE_SIZE := $(shell cat $(CODE) | wc -c)
RULE_SIZE := $(shell cat $(CODE) | perl -pe 's/[;{}]\s//g' | perl -pe 's/\s//g' | wc -c)

.PHONY: static-test
static-test:
	@echo "code size $(CODE_SIZE) / 4096"
	@test $(CODE_SIZE) -le 4096
	@echo "rule size $(RULE_SIZE) / 2048"
	@test $(RULE_SIZE) -le 2048
	@echo "testing 'build' script"
	@rm -rf prog.c prog
	@cp $(CODE) prog.c
	@./build
	@test -e prog
	@rm -rf prog.c prog
	@echo "testing README.markdown"
	@Markdown.pl README.markdown >/dev/null

$(DATA): $(DNAME)
	./$(DNAME)

$(NAME): static-test $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(LDFLAGS)

$(DNAME): $(DCODE) $(CODE)
	$(CC) $(CFLAGS) -o $@ $(DCODE) $(LDFLAGS)

.PHONY: clean
clean:
	rm -rf $(NAME) $(OBJ)
	rm -rf prog.c prog
	rm -rf $(DNAME)
	rm -rf $(DNAME).dSYM
	rm -rf $(DATA)
