[ $# -eq 0 ] && directorys=`pwd` || directorys=$@

linkchk () {
for element in $1/*; do
    [ -h "$element" -a ! -e "$element" ] && echo \"$element\"
    [ -d "$element" ] && linkchk $element
    done
}

for directory in $directorys; do
    if [ -d $directory ]
        then linkchk $directory
        else
            echo "$directory не является каталогом"
    fi
done

exit 0
