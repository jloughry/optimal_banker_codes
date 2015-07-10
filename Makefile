target = $(generated_pdf_file)

order = 5

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

rm = rm -f
cc = cc -Wall
edit = vi

all:: $(target)

$(generated_pdf_file): $(generator)
	./$(generator) $(order) > $(generated_dot_file)
	dot -T pdf $(generated_dot_file) -o $(generated_pdf_file)

$(generator): $(generator_sources) Makefile
	gcc -Wall -Wextra -o $@ $<
	mv $(generator_compiled) $@

clean::
	$(rm) $(target) $(generator) *.stackdump \
		$(generated_dot_files) $(generated_pdf_files) \
		$(bibtex_file) typescript

test: $(target)
	./$(target) $(order)

vi:
	$(edit) $(generator_source)

include common.mk

