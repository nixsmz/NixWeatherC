# NixWeatherC
Stop doing things with C++. C++ is trash. This an astro-simple and ultra-light C application fetching weather informations.
Ideal on embedded systems.

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
