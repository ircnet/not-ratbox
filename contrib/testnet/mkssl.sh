#!/bin/bash

rm -f *.conf *.cert *.key

openssl dhparam -out dh.pem 1024

function mkselfsigned()
{
openssl req -subj /CN=$1 -new -newkey rsa:384 -days 365 -nodes -x509 -keyout $1.key -out $1.cert
}

function sign()
{
openssl req -subj /CN=$1 -new -newkey rsa:384 -days 365 -nodes -keyout $1.key -out $1.req
openssl x509 -req -in $1.req -CA $2.cert -CAkey $2.key -out $1.cert -set_serial 0
}


mkselfsigned hub1.testa
mkselfsigned hub2.testa

mkselfsigned hub1.testb

# hub2 is not self-signed, it's ca is hub1
sign hub2.testb hub1.testb

cp hub1.testa.cert .hub1.testa.cert
cp hub2.testa.cert .hub2.testa.cert


# testa and testb know each other's self-signed certs
cat hub1.testa.cert >> hub2.testa.cert
cat .hub2.testa.cert >> hub1.testa.cert

# hub1.testa knows only about hub1.testb cert (which is CA for hub2.testb, so hub2.testb will be able to connect to hub1.testa as well)
cat hub1.testb.cert >> hub1.testa.cert

# hub2.testb must know about hub1, being signed by it is not enough
cat hub1.testb.cert >> hub2.testb.cert

# hub{12}.testb knows about both hub{12}.testa, however hub2.testb->hub2.testa will not work as hub2.testa does not know about hub1.testb CA
cat .hub1.testa.cert >> hub1.testb.cert
cat .hub2.testa.cert >> hub1.testb.cert
cat .hub1.testa.cert >> hub2.testb.cert
cat .hub2.testa.cert >> hub2.testb.cert

rm -f .hub*.cert

# make everyone connect to everyone to check x509 restrictions work

./mkrb.sh "hub1.testa" 6667 "
hub2.testa:6668
*.testb:6607:*.testa
*.testb:6608:*.testa
"

./mkrb.sh "hub2.testa" 6668 "
hub1.testa:6667
*.testb:6607:*.testa
*.testb:6608:*.testa
"

./mkrb.sh "hub1.testb" 6607 "
hub2.testb:6608
*.testa:6667:*.testb
*.testa:6668:*.testb
"

./mkrb.sh "hub2.testb" 6608 "
hub1.testb:6607
*.testa:6667:*.testb
*.testa:6668:*.testb
"


