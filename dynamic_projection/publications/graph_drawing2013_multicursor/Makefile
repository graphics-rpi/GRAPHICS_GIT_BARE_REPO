# make -f Makefile

R = GraphDrawing2013
#STY = *.sty
BIB = *.bib


pdf: $(R).pdf

$(R).bbl: $(BIB)
	pdflatex $(R)
	bibtex $(R)
	pdflatex $(R)
	bibtex $(R)


$(R).pdf: *.tex  $(FIG) $(R).bbl $(BIB) 
	pdflatex $(R).tex
	pdflatex $(R).tex

clean:
	rm -f $(R).log $(R).aux $(R).bbl $(R).blg $(R).out
