#! /bin/csh

setenv DIALOG dialog

if ($# != 1) then
	echo "start.csh <fileList>"
	exit 1
endif

pushd ../bin
$DIALOG --title "MESSAGE BOX" --clear \
        --msgbox "Press ENTER to start the experiment\
							Or ESC to exit." 10 41
if ($? == 255 ) then
	exit 1
endif

foreach f (`cat $1`)
	echo $f
	./P300Stimulator ${f}
	$DIALOG --title "MESSAGE BOX" --clear \
        --msgbox "Press ENTER to continue the experiment.\
							Or ESC to exit." 10 41
	if ($? == 255 ) then
		exit 1
	endif
end
pushd

