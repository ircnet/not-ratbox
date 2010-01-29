#!/bin/sh

. ./config

chmod +x *.sh

# suffixes of masked clusters
sufa="${OPERNAME}a"
sufb="${OPERNAME}b"
sufc="${OPERNAME}c"

# created topology is like
#	hub1.sda [6667]------.------hub1.sdb [6607]
#	|			/ \	|
#	hub2.sda [6668]-'   `----hub2.sdb [6608]-------hub1.sub.sdb[6617]---hub2.sub.sdb[6618]----leaf1.sub.sdb[6619]
#	|		       		|                              \__leaf2.sub.sdb[6620]
#	leaf.sda [6669]	 	*.leaf.sdb [6609]       
#				                                     
#### left part
./mkrb.sh "hub1.$sufa" 6667 "
*.$sufb:6607:*.$sufa
*.$sufb:6608:*.$sufa
hub2.$sufa:6668
leaf.$sufa:
"

./mkrb.sh "hub2.$sufa" 6668 "
*.$sufb:6607:*.$sufa
*.$sufb:6608:*.$sufa
hub1.$sufa:6667
leaf.$sufa:
"

./mkrb.sh "leaf.$sufa" 6669 "
hub1.$sufa:6667
hub2.$sufa:6668
"


#### right part
./mkrb.sh "hub1.$sufb" 6607 "
*.$sufa:6667:*.$sufb
*.$sufa:6668:*.$sufb
hub2.$sufb:6608
*.leaf.$sufb:6609
"

./mkrb.sh "hub2.$sufb" 6608 "\
*.$sufa:6667:*.$sufb
*.$sufa:6668:*.$sufb
hub1.$sufb:6607
*.leaf.$sufb:6609
*.sub.$sufb:6617:
"

./mkrb.sh "single.leaf.$sufb" 6609 "
hub1.$sufb:6607:*.leaf.$sufb
hub2.$sufb:6608:*.leaf.$sufb
"

#### sub.sdb part
./mkrb.sh "hub1.sub.$sufb" 6617 "
hub2.$sufb:6608:*.sub.$sufb
hub2.sub.$sufb:6618
leaf1.sub.$sufb:
leaf2.sub.$sufb:
"

./mkrb.sh "hub2.sub.$sufb" 6618 "
hub1.sub.$sufb:6617
leaf1.sub.$sufb:
leaf2.sub.$sufb:
"

./mkrb.sh "leaf1.sub.$sufb" 6619 "
hub2.sub.$sufb:6618
"

./mkrb.sh "leaf2.sub.$sufb" 6620 "
hub1.sub.$sufb:6617
"

