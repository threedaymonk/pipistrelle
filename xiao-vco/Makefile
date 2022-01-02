GERBERS = $(wildcard gerbers/*)

.PHONY: clean

xiao-dvco.zip: $(GERBERS)
	zip -j .$@.tmp $^ && mv .$@.tmp $@

clean:
	rm -f xiao-dvco.zip
