
# make -f Makefile

R = photons_on_tt
STY = *.sty


pdf: $(R).pdf

$(R).bbl: $(R).bib
	pdflatex $(R)
	bibtex $(R)
	pdflatex $(R)
	bibtex $(R)


#$(R).pdf: *.tex $(STY) $(FIG) $(R).bib
$(R).pdf: *.tex $(STY) $(FIG) $(R).bbl $(R).bib
	pdflatex $(R)

clean:
	rm -f $(R).log $(R).aux $(R).bbl $(R).blg $(R).out
