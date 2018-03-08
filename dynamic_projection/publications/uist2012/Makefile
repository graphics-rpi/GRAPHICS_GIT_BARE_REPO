# make -f Makefile

R = paper
#STY = *.sty
BIB = *.bib


pdf: $(R).pdf

$(R).bbl: $(BIB)
	pdflatex $(R)
	bibtex $(R)
	pdflatex $(R)
	bibtex $(R)


#$(R).pdf: *.tex $(STY) $(FIG) $(R).bib
$(R).pdf: *.tex  $(FIG) $(R).bbl $(BIB) 
#$(STY)
	pdflatex $(R)

clean:
	rm -f $(R).log $(R).aux $(R).bbl $(R).blg $(R).out
