target = $(generator)

order = 5

# DEBUG_FLAGS = -DDEBUG
DEBUG_FLAGS =

GCC_FLAGS = -Wall -Wextra
GDB_FLAGS = -g
LINKER_FLAGS = -lgmp

generator = generate_programme
generated_file = order-$(order)_graph_generated
generator_source = $(generator).c
generator_header = $(generator).h
generator_compiled = $(generator).exe
generator_sources = $(generator_source) $(generator_header)
generated_dot_file = $(generated_file).dot
generated_pdf_file = $(generated_file).pdf

generated_dot_files = order-*_graph_generated.dot
generated_pdf_files = order-*_graph_generated.pdf
debug_symbol_files = $(generator).dSYN
checkpoint_file = checkpoint.xml

.PHONY: test clean $(generated_pdf_file)

rm = rm -fv
edit = vi

all:: $(target)

$(generated_pdf_file): $(generator)
	./$(generator) -1g $(order) > $(generated_dot_file)
	dot -T pdf $(generated_dot_file) -o $(generated_pdf_file)

#
# In Cygwin, it's necessary to put -lgmp last on the command line.
#

$(generator): $(generator_sources) Makefile
	gcc $(GDB_FLAGS) $(GCC_FLAGS) $(DEBUG_FLAGS) -o $@ $< $(LINKER_FLAGS)
	mv $(generator_compiled) $@

test: $(generated_pdf_file)

clean::
	$(rm) $(target) $(generator) *.stackdump \
		$(generated_dot_files) $(generated_pdf_files) \
		$(bibtex_file) typescript \
		$(checkpoint_file)

	$(rm) -r $(debug_symbol_files)

vi:
	$(edit) $(generator_source)

header:
	$(edit) $(generator_header)

include common.mk

