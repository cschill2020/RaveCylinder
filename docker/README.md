# RaveCylinder/docker

Hopefully to aid local development, this contains a Docker setup. Assumes you
have the Docker client already installed; for Mac I recommend [Docker
Desktop](https://www.docker.com/products/docker-desktop/).

## Step 1: Build and compile

```bash
# build the docker image, which includes installing/building the necessary deps
./build

# then, this compile will do a clean build each time
#
# this calls the script [`container-cmake.sh`]() from within the container
./compile

# confirm: the binary exists, accessible from the host OS (i.e. outsider the
# container)
# note: this is a Linux binary, so it (most likely?) will NOT run on the host
# (macos)
ls RaveCylinder
```

## Step 2: Run the web server

```bash
# use the convenience command to run the server in a container
./rave-server

# now you can load the main page in the browser
open localhost:8088

# TODO: to kill container, you'll need to CTRL-C twice; this is because the
# process doesn't yet handle SIGKILL
```

## Additional Notes

```bash
# for convenience/debugging, there's a script to run a shell from the image
./shell

# inside the container, the repo is mounted on /local
> pwd     # /local
> ls      # will see the local repo
> cmake . # can do stuff like running cmake
```
