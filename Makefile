SUBDIRS = lars_reactor lars_dns lars_reporter lars_loadbalance_agent

.PHONY: all

all:
	@list='$(SUBDIRS)'; for subdir in $$list; do \
		echo "Clean in $$subdir";\
		$(MAKE) -C $$subdir;\
	done

.PHONY: clean

# 删除所有list下对应的src文件中的.o文件
rmo:
	@list='$(SUBDIRS)'; for subdir in $$list; do \
		if [ -d $$subdir/src ]; then \
			echo "Removing .o files in $$subdir/src";\
			rm -f $$subdir/src/*.o;\
		fi \
	done
clean:
	@echo Making clean
	@list='$(SUBDIRS)'; for subdir in $$list; do \
		echo "Clean in $$subdir";\
		$(MAKE) -C $$subdir clean;\
	done