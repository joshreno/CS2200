# Authored by Christopher Tam for Georgia Tech's CS 2200
SUBMIT_SUFFIX = -datapath
SUBMIT_FILES = GT-2200.circ microcode.xlsx

all: ;

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
