
At startup send mqtt events top 127.0.0.1, channel: ets2/telematic

view events with:

    mosquitto_sub -h 127.0.0.1 -p 1883 -t '#'
