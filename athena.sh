#!/bin/sh
echo "Athena Starting..."
echo "                   (c) 2003 Athena Project."
echo "                URL:http://project-yare.de/"
echo ""
echo "I gonna show debug infomations,"
echo " because this is not release."
echo ""
echo "checking..."

if [ ! -f ./account.txt ]; then
    echo "0	s1	p1	-	S	0" > account.txt
    echo "1	s2	p2	-	S	0" >>account.txt
    echo "2	s3	p3	-	S	0" >>account.txt
    echo "3	s4	p4	-	S	0" >>account.txt
    echo "4	s5	p5	-	S	0" >>account.txt
fi
if [ ! -x ./login-server ]; then
    echo "login-server does not exist, or can't run."
    echo "Stoped, Check your compile."
    exit 1;
fi

if [ ! -x ./char-server ]; then
    echo "char-server does not exist, or can't run."
    echo "Stoped, Check your compile."
    exit 1;
fi

if [ ! -x ./map-server ]; then
    echo "map-server does not exist, or can't run."
    echo "Stoped, Check your compile."
    exit 1;
fi
echo "Done."
echo "Looks good. Good Luck!"
./login-server conf/login_athena.conf &
./char-server conf/char_athena.conf conf/inter_athena.conf &
./map-server conf/map_athena.conf conf/battle_athena.conf &
echo ""
echo "Now Started Athena."
