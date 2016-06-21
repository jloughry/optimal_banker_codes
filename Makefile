target = $(generator)

order = 5

# DEBUG_FLAGS = -DDEBUG
DEBUG_FLAGS =

GCC_FLAGS = -Wall -Wextra -pedantic
GDB_FLAGS = -g
LINKER_FLAGS = -I /usr/local/include -L /usr/local/lib -lgmp

generator = generate_programme
generated_file = order-$(order)_graph_generated
generator_source = $(generator).c
generator_header = $(generator).h
generator_compiled = $(generator).exe
generator_sources = $(generator_source) $(generator_header) $(version)
generated_dot_file = $(generated_file).dot
generated_pdf_file = $(generated_file).pdf

generated_dot_files = order-*_graph_generated.dot
generated_pdf_files = order-*_graph_generated.pdf
debug_symbol_files = $(generator).dSYN
checkpoint_file = checkpoint.xml

build_counter = counters/build_counter.txt
version = version.h

.PHONY: test clean $(generated_pdf_file)

rm = rm -fv
edit = vi

all:: $(target) ## (default target)

$(generated_pdf_file): $(generator)
	./$(generator) -1g $(order) > $(generated_dot_file)
	dot -T pdf $(generated_dot_file) -o $(generated_pdf_file)

#
# In Cygwin, it's necessary to put -lgmp last on the command line.
#

$(generator): $(generator_sources) Makefile
	gcc $(GDB_FLAGS) $(GCC_FLAGS) $(DEBUG_FLAGS) -o $@ $< $(LINKER_FLAGS)
	mv $(generator_compiled) $@
	@echo $$(($$(cat $(build_counter)) + 1)) > $(build_counter)
	@echo "Build `cat $(build_counter)`"

test: $(generated_pdf_file) ## Run tests.

help: ## Show this help.
	@grep -hP '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | sort \
	| awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-20s\033[0m %s\n", $$1, $$2}'

clean:: ## Remove any files that can be re-generated automatically.
	$(rm) $(target) $(generator) *.stackdump \
		$(generated_dot_files) $(generated_pdf_files) \
		$(bibtex_file) typescript \
		$(checkpoint_file)

	$(rm) -r $(debug_symbol_files)

commit:: ## Update source at GitHub.
	@echo "#define VERSION $$(($$(cut -d ' ' -f 3 $(version)) + 1))" > $(version)
	@echo "Version `cut -d ' ' -f 3 $(version)`"

vi:	## Edit the main C source.
	$(edit) $(generator_source)

header:	## Edit the main C header file.
	$(edit) $(generator_header)

include common.mk

