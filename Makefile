NAME := death
CODE := $(NAME).c
OBJ  := $(CODE:.c=.o)

DNAME := $(NAME)-data
DCODE := $(DNAME).c
DATA  := dead.dat splash.dat sprites.dat

TNAME   := $(NAME)-test
TCODE   := $(TNAME).c

CC      := clang
CFLAGS  := -ggdb -std=c99 -Wall -pedantic-errors -I/usr/X11R6/include -L/usr/X11R6/lib -D_BSD_SOURCE
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

$(DATA): $(DNAME)
	./$(DNAME)

$(NAME): static-test test $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(LDFLAGS)

$(TNAME): $(DATA) $(TCODE) $(CODE)
	$(CC) $(CFLAGS) -o $@ $(TCODE) $(LDFLAGS) -lcheck

$(DNAME): $(DCODE) $(CODE)
	$(CC) $(CFLAGS) -o $@ $(DCODE) $(LDFLAGS)

.PHONY: clean
clean:
	rm -rf $(NAME) $(OBJ)
	rm -rf $(TNAME)
	rm -rf prog.c prog
	rm -rf $(TNAME).dSYM
	rm -rf $(DNAME)
	rm -rf $(DNAME).dSYM
	rm -rf $(DATA)
