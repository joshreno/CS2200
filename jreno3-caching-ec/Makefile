# Authored by Christopher Tam for Georgia Tech's CS 2200
TARGET = cachesim

CC     = gcc
CFLAGS = -Wall -Wextra -Wsign-conversion -Wpointer-arith -Wcast-qual -Wwrite-strings -Wshadow -Wmissing-prototypes -Wpedantic -Wwrite-strings -g -std=gnu99

LFLAGS =

SRCDIR = src
INCDIR = $(SRCDIR)
OBJDIR = obj
BINDIR = .

SUBMIT_SUFFIX = -caching-ec
SUBMIT_FILES  = $(SRC) $(INC) Makefile

SRC := $(wildcard $(SRCDIR)/*.c)
INC := $(wildcard $(INCDIR)/*.h)
OBJ := $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRC))

INCFLAGS := $(patsubst %/,-I%,$(dir $(wildcard $(INCDIR)/.)))

.PHONY: all
all:
	@$(MAKE) release && \
	echo "$$(tput setaf 3)$$(tput bold)Note:$$(tput sgr0) this project compiled with release flags by default. To compile for debugging, please use $$(tput setaf 6)$$(tput bold)make debug$$(tput sgr0)."

.PHONY: debug
debug: CFLAGS += -Og -DDEBUG
debug: $(TARGET)

.PHONY: release
release: CFLAGS += -mtune=native -O2
release: $(TARGET)

.PHONY: clean
clean:
	@rm -rf $(OBJDIR)
	@rm -f $(BINDIR)/$(TARGET)

.PHONY: check-username
check-username:
	@if [ -z "$(GT_USERNAME)" ]; then \
		echo "Before running 'make submit', please set your GT Username in the environment"; \
		echo "Run the following to set your username: \"export GT_USERNAME=<your username>\""; \
		exit 1; \
	fi

.PHONY: submit
submit: check-username
	@(tar zcfh $(GT_USERNAME)$(SUBMIT_SUFFIX).tar.gz $(SUBMIT_FILES) && \
	echo "Created submission archive $$(tput bold)$(GT_USERNAME)$(SUBMIT_SUFFIX).tar.gz$$(tput sgr0).") || \
	(echo "$$(tput bold)$$(tput setaf 1)Error:$$(tput sgr0) Failed to create submission archive." && \
	rm -f $$name$(SUBMIT_SUFFIX).tar.gz)


$(TARGET): $(OBJ)
	@mkdir -p $(BINDIR)
	@$(CC) $(CFLAGS) $(INCFLAGS) -o $(BINDIR)/$@ $^ $(LFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(OBJDIR)
	@$(CC) $(CFLAGS) -c -o $@ $<
