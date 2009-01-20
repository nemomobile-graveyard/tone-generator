#:bin/bash

echo ""

for i in 1 2 3 4 5 6 7 8 9 10 ; do
    echo -n "$i "

    dbus-send --system --type=method_call --dest=com.Nokia.Telephony.Tones \
	/com/Nokia/Telephony/Tones com.Nokia.Telephony.Tones.StartEventTone \
	uint32:66 int32:0 uint32:3000

    sleep 2

    dbus-send --system --type=method_call --dest=com.Nokia.Telephony.Tones \
	/com/Nokia/Telephony/Tones com.Nokia.Telephony.Tones.StopTone

    sleep 3

done

echo ""
