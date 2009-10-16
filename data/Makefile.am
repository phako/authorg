schemadir = @GCONF_SCHEMA_FILE_DIR@
schema_DATA = authorg.schemas

logodir = ${datadir}/authorg/
logo_DATA = \
			authorg.png

if SCHEMAS_INSTALL
install-data-local:
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(srcdir)/$(schema_DATA)
endif

EXTRA_DIST = \
			 $(schema_DATA) $(logo_DATA)
