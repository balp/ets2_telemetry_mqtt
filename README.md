Eurotruck Simulator 2 and Americal Truck Simulator MQTT Gateway
===============================================================

This is a ATS/ETS2 Telematic to MQTT gateway.

I have a simple python dashboard at: 

    https://github.com/balp/ets2_dash

Currenty version have no config and will assume there is an mqtt server at 127.0.0.1
where it will publish json formated telemetrics informations on the channels starting
with ets2/ the following challels are used at the moment:

* etc2/data                   - Telematic frame data
* ets2/game                   - Connected game info, e.g. version and name
* ets2/info                   - Game status, e.g. Paused or Started
* ets2/info/config/#          - One per ETS2 config channel at the moment
* ets2/info/config/substances -
* ets2/info/config/job        -
* ets2/info/config/controls   -
* ets2/info/config/hshifter   -
* ets2/info/config/truck      -
* ets2/info/config/trailer    -


Concourse - CI
--------------
Install local version:

    wget https://concourse-ci.org/docker-compose.yml
    docker-compose up -d

Possibly change config a bit, before the up command.

    CONCOURSE_ADD_LOCAL_USER: <<username>>:<<password>>
    CONCOURSE_MAIN_TEAM_LOCAL_USER: <<username>>


Login:

    fly --target laurana-main login --team-name main --concourse-url http://localhost:8080/


zsh command line completion:

    source <(fly completion --shell=zsh)

Deploy a new pipeline:

    fly -t laurana-main set-pipeline --pipeline ets2_telemetry_mqtt --config pipeline.yml -l secrets.yml


Run a CI build on your local code, with out committing:

     fly -t laurana-main execute --config pipeline.yml --input ets2_telemetry_mqtt=. --config tasks/cmake-linux.yml -l secrets.yml
