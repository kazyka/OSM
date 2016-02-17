STUDENTS=graham_knuth_patashnik
ASSIGNMENT=1
VERSION=1

TAR_GZ=$(STUDENTS)-G$(ASSIGNMENT)-v$(VERSION).tar.gz

all: $(TAR_GZ)

$(TAR_GZ):
	make -C src/queue/ clean
	make -C src/kudos/kudos/ clean
	make -C src/kudos/userland/ clean
	tar cvzf $(TAR_GZ) src/queue/ \
		src/kudos/kudos/ \
		src/kudos/userland/ \
		src/kudos/yams.conf

clean:
	rm -rf $(TAR_GZ)

.PHONY: all clean
