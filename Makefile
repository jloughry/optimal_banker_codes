order = 5

target = optimal_gray_code_banker_sequence

chart_source = order-5_graph.dot
pdf_file = order-5_graph.pdf
generator = generate_binary
generated_file = order-$(order)_graph_generated
generator_source = $(generator).c
generated_dot_file = $(generated_file).dot
generated_pdf_file = $(generated_file).pdf

rm = rm -f
cc = cc -Wall
edit = vi

all:: $(target) $(pdf_file) $(generator)

$(target): $(target).c Makefile
	$(cc) -o $@ $<
	mv $@.exe $@

$(generator): $(generator_source) Makefile
	gcc -Wall -Wextra -o $@ $<
	mv $(generator).exe $@
	./$(generator) 5 > $(generated_dot_file)
	dot -T pdf $(generated_dot_file) -o $(generated_pdf_file)

clean::
	$(rm) $(target) $(pdf_file) $(generator) *.stackdump $(generated_dot_file) $(generated_pdf_file)

test: $(target)
	./$(target) 5

vi:
	$(edit) $(generator_source)

$(pdf_file): $(chart_source)
	dot -T pdf $(chart_source) -o $(pdf_file)

include common.mk

