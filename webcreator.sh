#!/bin/bash

#function to make sets of q and f
make_set()
{
	cur_site=$1
	echo Received $cur_site
	#finds first number of string , site id
	sid=`echo "$cur_site" | grep -oP '^[^0-9]*\K[0-9]+'`
	setF=()
	setQ=()
	local i=0
	local y=0
	for element in ${names[@]}
	do
		tmpid=`echo "$element" | grep -oP '^[^0-9]*\K[0-9]+'`
		if [ "$sid" -eq "$tmpid" ]; then
			if [ "$cur_site" != "$element" ]; then
				setF[$i]=$element
				((++i))
			fi
		else
			setQ[$y]=$element
			((++y))
		fi
	done
	let "q=($w/2)+1"
	let "f=($p/2)+1"
	#echo Q is $q F is $f
	y=0
	#let "rr=${#setQ[@]}-1"
	#echo RR $rr
	#echo SETQ is
	#echo ${setQ[@]}
	if [ "$q" -ne 1 ]; then
		for ((i=0;i<$q;i++))
		do
			let "rand=((0+ $RANDOM % ${#setQ[@]}))"
			while [ -z "${setQ[$rand]}" ]
			do
				#echo LOOPA ${setQ[@]} 
				let "rand=((0+ $RANDOM % ${#setQ[@]}))"
			done
			#pare ton arithmo tou
			tmpid=`echo "${setQ[$rand]}" | grep -oP '^[^0-9]*\K[0-9]+'`
			combined[$y]=../site${tmpid}/${setQ[$rand]}
			unset 'setQ[$rand]'
			#cho RANDQ IS $rand
			#echo ${combined[$y]} or ${setQ[$rand]}
			((++y))
		done
	fi
	#echo SETF ISS ${#setF[@]}
	let "rr=${#setF[@]}"
	#echo \*\*RR $rr
	for ((i=0;i<$f;i++))
	do
		let "rand=((0+ $RANDOM % $rr))"
		if [ "$rr" -ne 1 ]; then
			while [ -z "${setF[$rand]}" ]
			do
				#echo ${setF[@]} 
				#echo LOOP $rand
				let "rand=((0+ $RANDOM % $rr))"
				#exit 1
			done
			#combined[$y]=site${sid}/${setF[$rand]}
			combined[$y]=${setF[$rand]}
			echo "COMBINED1 ${combined[$y]}"
			unset 'setF[$rand]'
		else									#if setF has only one element then
			combined[$y]=${setF[0]}				#take the element
			((++y))								#and 
			#combined[$y]=site${sid}/${cur_site}			#take page that we work on
			combined[$y]=${cur_site}
			echo "COMBINED2 ${combined[$y]}"
			break
		fi
		#echo RANDF IS $rand
		# combined[$y]=${setF[$rand]}
		# unset 'setF[$rand]'
		#echo ${combined[$y]}
		((++y))
	done
	#randQ
	#echo combined length ${#combined[@]}
}



#check number of arguments
if [ "$#" -ne "4" ]; then
	echo Not enough args
fi
#path of directory
root_dir="$1"
#check if directory exists and is empty
#if not , purge dir 
if [ ! -d "$root_dir" ]; then
	echo Directory not found
	exit 1
fi
if [ "$(ls -A $root_dir)" ]; then
	echo \# Warning: directory is full , purging ...
	rm -rf $root_dir/*
fi
#take rest of arguments
text_file="$2"
filelines=`cat $text_file`
w=$3
p=$4
#check w , p if integers 
re=^[0-9]+$
if ! [[ $w =~ $re ]]; then
	echo Not an integer : $w
	exit 2
fi
if ! [[ $p =~ $re ]]; then
	echo Not an integer : $p
	exit 2
fi
#check for number of lines
countlines=`wc -l <$text_file`
if [ $countlines -lt 10000 ]; then
	echo Text has less than 10k lines
	exit 3
fi
cd "$root_dir"
countW=0
countP=0
#generate names for sites
for ((i=0; i<= ($w*$p)-1;i++))
do
	if [ $countP -eq $p ]; then
		let "countP=0"
		let "countW += 1" 
		echo 
	fi
	names[${i}]=page$countW\_$RANDOM
	#echo ${names[${i}]}
	let "countP += 1"
	#touch site$i/page$i\_$RANDOM.html
done

#for every page make q and f sets and create html file
counter=0
for ((i=0;i<$w;i++))
do
	echo "# Creating web site $i ..."
	mkdir site$i
	cd site$i
	for ((y=0;y<$p;y++))
	do
		#echo Work for w is $i and page ${names[$counter]}
		#pwd
		combcounter=0
		#generate k , m
		let "toplimit=${countlines}-2000"
		k=`shuf -i 2-$toplimit -n 1`
		m=`shuf -i 1001-1999 -n 1` 
		# k=500
		# m=1500
		#echo K is  $k  M is $m
		#give as argument page we work on
		make_set ${names[$counter]}
		#calculate m/f+q
		let "package=$m/${#combined[@]}"
		# echo LINKS ${#combined[@]}
		# echo ${combined[@]}
		pVal=$k
		#create html file
		echo "#   Creating page ${names[$counter]}.html with $m lines starting at line $k"
		touch ${names[$counter]}.html
		#html headers
		echo "<!DOCTYPE html>" >> ${names[$counter]}.html
		echo "<html>" >> ${names[$counter]}.html
		echo "	<body>" >> ${names[$counter]}.html
		
		lineswritten=0
		let "linesleft=$m % ${#combined[@]}"
		while [ "$lineswritten" -ne "$m" ]
		do
			if [ "${linesleft}" -eq 0 ]; then
				let "start=${pVal}+${lineswritten}"
				let "end=${pVal}+${lineswritten}+(${package}-1)"
				sed -n "${start},${end}p" $text_file >> ${names[$counter]}.html
				let "lineswritten = lineswritten + package"
			else			#if m mod (f+q) != 0 take one extra line in every loop 
				let "start=${pVal}+${lineswritten}"
				let "end=${pVal}+${lineswritten}+${package}"
				sed -n "${start},${end}p" $text_file >> ${names[$counter]}.html
				let "lineswritten = lineswritten + package + 1"
				((--linesleft))
			fi
			#change line with <br>
			echo "<br>" >> ${names[$counter]}.html
			echo "#   Adding link to ${root_dir}site${i}/${names[$counter]}.html"
			#echo '	<a href='${1}${combined[$combcounter]}.html' >LINK</a>' >> ${names[$counter]}.html
			#evgala to keno apo to >
			echo '	<a href='${combined[$combcounter]}.html'>LINK</a>' >> ${names[$counter]}.html
			((++combcounter))
		done
		#close html headers
		echo "	</body>" >> ${names[$counter]}.html
		echo "</html>" >> ${names[$counter]}.html
		((++counter))
		temporary=${names[$counter]}.html
	done
	cd ..
done
echo "# Done."

pos=0
for element in ${names[@]}
do
	#find all occurences of pageX as link in all files
	var1=`grep -rnw ${root_dir} -e $element | wc -l`
	tempid=`echo "$element" | grep -oP '^[^0-9]*\K[0-9]+'`
	#find occurences of pageX in pageX.html
	var2=`grep -e $element ${root_dir}site$tempid/$element.html | wc -l`
	#see if there are other occurences except pageX.html
	let "var=var1-var2"
	if [ "${var}" -gt 0 ]; then
		inlinks[$pos]=1
		((++pos))
	fi 
done

if [ "${#inlinks[@]}" -eq "${#names[@]}" ]; then
	echo "# All pages have at least one incoming link"
else
	echo "# Not all pages have at least one in incoming link"
fi
