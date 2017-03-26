UCMX_DIR 	?= $(realpath lib/unicore-mx)
APP_SRC		:= src/
all: build

build: unicore-mx app

unicore-mx:
	$(Q)if [ ! "`ls -A $(UCMX_DIR)`" ] ; then \
		printf "######## ERROR ########\n"; \
		printf "\tunicore-mx is not initialized.\n"; \
		printf "\tPlease run:\n"; \
		printf "\t$$ git submodule init\n"; \
		printf "\t$$ git submodule update\n"; \
		printf "\tbefore running make.\n"; \
		printf "######## ERROR ########\n"; \
		exit 1; \
		fi
	$(Q)$(MAKE) -C $(UCMX_DIR)

app: unicore-mx
	$(Q)$(MAKE) --directory=$(APP_SRC) UCMX_DIR=$(UCMX_DIR)
