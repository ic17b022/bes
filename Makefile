##
## @file Makefile
## myfind - a simplified version of find(1)
## Beispiel 1
##
## @author Manuel Seifner	 <ic17b022@technikum-wien.at>
## @author Oliver Safar		 <ic17b077@technikum-wien.at>
## @date 2018/04/55
##
## @version 1.0 $
##
##

##
## ------------------------------------------------------------- variables --
##

CC=gcc52
CFLAGS=-Wall -Werror -Wextra -Wstrict-prototypes -Wformat=2 -pedantic -fno-common -ftrapv -O3 -g -std=gnu11
CP=cp
CD=cd
MV=mv
GREP=grep
DOXYGEN=doxygen

OBJECTS=myFind.o

EXCLUDE_PATTERN=footrulewidth

##
## ----------------------------------------------------------------- rules --
##

%.o: %.c
	$(CC) $(CFLAGS) -c $<

##
## --------------------------------------------------------------- targets --
##

.PHONY: all
all: myfind

myfind: $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

.PHONY: clean
clean:
	$(RM) *.o *~ myfind

.PHONY: distclean
distclean: clean
	$(RM) -r doc

doc: html pdf

.PHONY: html
html:
	$(DOXYGEN) doxygen.dcf

pdf: html
	$(CD) doc/pdf && \
	$(MV) refman.tex refman_save.tex && \
	$(GREP) -v $(EXCLUDE_PATTERN) refman_save.tex > refman.tex && \
	$(RM) refman_save.tex && \
	make && \
	$(MV) refman.pdf refman.save && \
	$(RM) *.pdf *.html *.tex *.aux *.sty *.log *.eps *.out *.ind *.idx \
	      *.ilg *.toc *.tps Makefile && \
	$(MV) refman.save refman.pdf

##
## ---------------------------------------------------------- dependencies --
##

##
## =================================================================== eof ==
##
