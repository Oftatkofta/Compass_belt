.PHONY: all clean

all: compass.pdf

compass.pdf: compass.tex compass.eps
	pdflatex compass.tex

clean:
	rm -f *.dvi *.pdf *.log *.aux
