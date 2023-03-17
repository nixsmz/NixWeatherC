# NixWeatherC
Dummy C application printing weather informations.

### Weather information
The program is using `curl` to fetch weather informations, as a `json` format.
Weather API source: https://open-meteo.com/

### Installing dependencies
```bash
apt install -y libcurl4-openssl-dev
```

### Starting demo code
```bash
make renew && ./main
```

### Author
@nix.dev
