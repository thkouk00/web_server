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
	echo Q is $q F is $f
	y=0
	let "rr=${#setQ[@]}-1"
	echo RR $rr
	for ((i=0;i<$q;i++))
	do
		let "rand=((0+ $RANDOM % (${#setQ[@]}-1)))"
		while [ -z "${setQ[$rand]}" ]
		do
			echo ${setQ[@]} 
			let "rand=((0+ $RANDOM % (${#setQ[@]}-1)))"
		done
		unset 'setQ[$rand]'
		echo RANDQ IS $rand
		combined[$y]=${setQ[$rand]}
		((++y))
	done
	echo SETF ISS ${#setF[@]}
	let "rr=${#setF[@]}"
	echo \*\*RR $rr
	echo HERE FINALLY ${setF[4]}
	for ((i=0;i<$f;i++))
	do
		let "rand=((0+ $RANDOM % $rr))"
		while [ -z "${setF[$rand]}" ]
		do
			#echo ${setF[@]} 
			echo $rand
			let "rand=((0+ $RANDOM % $rr))"
			#exit 1
		done
		unset 'setF[$rand]'
		echo RANDF IS $rand
		combined[$y]=${setF[$rand]}
		((++y))
	done
	#randQ
	echo combined length ${#combined[@]}
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
count=`wc -l <$text_file`
if [ $count -lt 10000 ]; then
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
	echo ${names[${i}]}
	let "countP += 1"
	#touch site$i/page$i\_$RANDOM.html
done

#for every page make q and f sets and create html file
counter=0
for ((i=0;i<$w;i++))
do
	mkdir site$i
	for ((y=0;y<$p;y++))
	do
		make_set ${names[$counter]}
		((++counter))
		#merge setF and setQ to one array
		# compined=(${setF[@]} ${setQ[@]})
		echo combined
		for element in ${combined[@]}
		do
			echo $element
		done
		echo
	done
done


# echo SETF
# for element in ${setF[@]}
# do
# 	echo $element
# done

# #put 1-35 lines to html site
# echo "<!DOCTYPE html>
# <html>
# 	<body>" >> ${names[0]}.html
# sed -n 1,35p $text_file >> ${names[0]}.html 
# touch ${names[10]}.html
# #mkdir site0
# echo "Kalispera sas" >> ${names[10]}.html
# #mv ${names[10]}.html site0
# # if [  ]; then

# # fi
# #rand num between 1-4
# #echo $((1+ RANDOM %4))
# echo ${names[44]} | grep -oP '^[^0-9]*\K[0-9]+'
# echo '	<a href='${names[10]}.html' >test</a>' >> ${names[0]}.html
# echo "	</body>
# </html>" >> ${names[0]}.html
# array[0]='randy'
# array[1]='orton'
# array[2]='raaas'
# echo Try to delete array
# echo ${#array[@]}
# array=()
# echo ${#array[@]}
# echo array deleted
# #delete=(randy)
# #echo ${array[@]/$delete}
# #array=("${array[@]:1}")
# echo ${array[@]}
# unset 'array[1]'
# echo ${array[@]}
# echo ${array[1]}
# # if [ -z "${array[0]}" ]; then
# # 	echo NULL re
# # fi
# i=0
# for element in ${array[@]}
# do
# 	array2[$i]=$element
# 	((++i))
# done
# # echo ${array[0]}
# # echo ${array[1]}
# # echo ${array[2]}
# echo ${array2[@]}
# echo ${array2[1]}
# echo ${#array2[@]}
# make_set ${names[@]}
# echo After functionn
# for element in ${names[@]}
# do
# 	echo $element
# done
# echo LEENGTH IS ${#names[@]}
# echo ${names[4]}
