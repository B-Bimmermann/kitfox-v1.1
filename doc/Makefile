PAPER = kitfox
VERSION=v1.0.0
TEX = $(wildcard *.tex)
FIGS = $(wildcard figures/*.pdf)

default: doc

doc:
	make $(PAPER).pdf

$(PAPER).pdf: $(TEX) $(BIB) $(FIGS)
	rm -rf *.glsdefs
	pdflatex $(PAPER)
	pdflatex $(PAPER)
	mv $(PAPER).pdf $(PAPER)-$(VERSION).pdf

clean:
	rm -f *.aux *.log *.out *.toc

distclean:
	rm -f *.aux *.log *.out *.toc $(PAPER)-$(VERSION).pdf $(PAPER).pdf

