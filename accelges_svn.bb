DESCRIPTION = "Openmoko Accelerometer-based Gestures"
SECTION = "openmoko/utilities"

PN = "accelges"
PV = "0.1.0+svnr${SRCREV}"
PR = "r4"

SRC_URI = "svn://accelges.googlecode.com;module=svn;proto=https"

S = "${WORKDIR}/svn"

inherit autotools

do_install_append() {
	install -d ${D}${sysconfdir}/ges/neo2
	install -c -D -m 644 ${S}/config/neo2/* ${D}${sysconfdir}/ges/neo2
	install -d ${D}${sysconfdir}/ges/neo3
	install -c -D -m 644 ${S}/config/neo3/* ${D}${sysconfdir}/ges/neo3
}

FILES_${PN} += ${datadir}

