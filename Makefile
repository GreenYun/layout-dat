.PHONY: all clean do-unar plconv do-decode

unar: unar.c
	cc -O2 -Wall -Wextra -o $@ $?

output:
	mkdir -p ./output

do-unar: unar output
	cd ./output && ./../unar 2>/dev/null && cd ..

plconv: unar do-unar
	find . -name "*.plist" -exec plutil -convert xml1 "{}" \;

decode-data: decode-data.c
	cc -O2 -Wall -Wextra -o $@ $?

do-decode: decode-data
	find . -name "data.txt" -exec ./$< "{}" \;

all: plconv do-decode

clean:
	rm -rf output
	rm unar
	rm decode-data
