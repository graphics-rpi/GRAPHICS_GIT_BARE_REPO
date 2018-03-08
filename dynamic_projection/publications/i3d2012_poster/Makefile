.PHONY: default clean

default: poster.pdf

poster.pdf: poster.tex refs.bib
	pdflatex poster.tex
	bibtex poster
	pdflatex poster.tex
	pdflatex poster.tex

clean:
	-rm -f poster.pdf *.log *.aux *.bbl *.blg *.out *~ *.exe