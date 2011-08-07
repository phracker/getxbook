#!/bin/sh
# See COPYING file for copyright and license details.
#
# Creates a project summary based on a doap file

test $# -ne 1 && echo "usage: $0 doap" && exit 1

rdf="$1"

q="PREFIX doap: <http://usefulinc.com/ns/doap#>
PREFIX foaf: <http://xmlns.com/foaf/0.1/>
SELECT ?home ?repo ?license ?maintainer ?maintainerhome ?lang ?repotype
WHERE {
?p a doap:Project;
   doap:homepage ?home;
   doap:repository ?r;
   doap:license ?license;
   doap:programming-language ?lang;
   doap:maintainer ?m.
?r a ?repotype;
   doap:location ?repo.
?m foaf:name ?maintainer;
   foaf:homepage ?maintainerhome.
}"

roqet -q -r csv -e "$q" -D /dev/stdin < $rdf | sed '/^Result/d' \
| while read r; do
	home=`echo $r | awk -F , '{print $2}'| sed -e 's/uri\(//' -e 's/\)//'`
	repo=`echo $r | awk -F , '{print $3}'| sed -e 's/uri\(//' -e 's/\)//'`
	licenseuri=`echo $r | awk -F , '{print $4}'| sed -e 's/uri\(//' -e 's/\)//'`
	maint=`echo $r | awk -F , '{print $5}'| sed -e 's/"//g'`
	mainthome=`echo $r | awk -F , '{print $6}'| sed -e 's/uri\(//' -e 's/\)//'`
	lang=`echo $r | awk -F , '{print $7}'| sed -e 's/"//g'`
	test "$licenseuri" = "http://www.gnu.org/licenses/gpl.html" && license="GPL"
	test "$licenseuri" = "http://www.gnu.org/licenses/agpl.html" && license="AGPL"
	test "$licenseuri" = "http://creativecommons.org/licenses/MIT/" && license="MIT"
	test "$licenseuri" = "http://www.isc.org/software/license" && license="ISC"
	repotype=`echo $r | awk -F , '{print $8}'| sed -e 's/uri\(//' -e 's/\)//'`
	test "$repotype" = "http://usefulinc.com/ns/doap#GitRepository" && repocmd="git clone"

	cat <<- _EOF_
- Project homepage: [$home]($home)
- Code repository: $repocmd $repo
- Maintainer: [$maint]($mainthome)
- Language: $lang
- License: [$license]($licenseuri)
- DOAP: [${home}doap.ttl](${home}doap.ttl)
_EOF_
done
