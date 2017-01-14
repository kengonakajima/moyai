while true
	a=`netstat -tan|grep 22222|grep EST`
    print a,"\n" if a!=""
    sleep 0.1
end
    