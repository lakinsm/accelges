DESCRIPTION = "Openmoko Gestures"
SECTION = "openmoko/utilities"

PN = "gestures"
PV = "0.1.0+svnr${SRCREV}"
PR = "r1"

SRC_URI = "svn://svn.projects.openmoko.org/svnroot;module=gestures;proto=https"

S = "${WORKDIR}/gestures"

inherit autotools

do_install_append() {
	install -d ${D}${sysconfdir}/ges/neo2
	install -c -D -m 644 ${S}/config/neo2/* ${D}${sysconfdir}/ges/neo2
	install -d ${D}${sysconfdir}/ges/neo3
	install -c -D -m 644 ${S}/config/neo3/* ${D}${sysconfdir}/ges/neo3
	install -d ${D}${datadir}/gesm
	install -c -D -m 644 ${S}/gesm/data/* ${D}${datadir}/gesm
}

