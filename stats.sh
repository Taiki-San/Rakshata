git log --shortstat | grep \(+\) | awk '{inserted+=$4; deleted+=$6} END {print "lines inserted:", inserted, "lines deleted:", deleted}'
