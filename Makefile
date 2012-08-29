NAME := death
CODE := $(NAME).c
OBJ  := $(CODE:.c=.o)

TNAME   := $(NAME)-test
TCODE   := $(TNAME).c

CC      := gcc
CFLAGS  := -ggdb -std=c89 -Wall -pedantic-errors -I/usr/X11R6/include -L/usr/X11R6/lib
LDFLAGS := -lX11

.PHONY: all
all: $(NAME)

CODE_SIZE := $(shell cat $(CODE) | wc -c)
RULE_SIZE := $(shell cat $(CODE) | perl -pe 's/[;{}]\s//g' | perl -pe 's/\s//g' | wc -c)

.PHONY: static-test
static-test:
	@echo "code size $(CODE_SIZE) / 4096"
	# @test $(CODE_SIZE) -le 4096
	@echo "rule size $(RULE_SIZE) / 2048"
	# @test $(RULE_SIZE) -le 2048
	@echo "testing 'build' script"
	@rm -rf prog.c prog
	@cp $(CODE) prog.c
	@./build
	@test -e prog
	@rm -rf prog.c prog
	@echo "testing README.markdown"
	@Markdown.pl README.markdown >/dev/null

.PHONY: test
test: $(TNAME)
	./$(TNAME)

$(NAME): static-test test $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(LDFLAGS)

$(TNAME): $(TCODE) $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(TCODE) $(LDFLAGS) -lcheck

.PHONY: clean
clean:
	rm -rf $(NAME) $(OBJ)
	rm -rf $(TNAME)
	rm -rf prog.c prog
	rm -rf $(TNAME).dSYM
