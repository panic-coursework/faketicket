Installation Guide
==================

You need a recent version of Node.js and Yarn. We recommend
using a Node.js version manager, like [NVM][nvm]. You can
install yarn with `sudo npm i yarn -g`.

[nvm]: https://github.com/nvm-sh/nvm

After you have Node.js and Yarn installed, install
`cmake-js` globally using yarn:

```bash
yarn global add cmake-js
export PATH="$PATH:~/.yarn/bin"
```

And then install dependencies:

```bash
cd faketicket
yarn
cd app
yarn
cd server
yarn
```

Build the app with:

```bash
cd /path/to/faketicket
bin/build-node
cd app
yarn generate
```

Finally, run the server:

```bash
cd server
PORT=8123 node .
```

Edit `PORT` to meet your needs. The server should be up and
running at this point. Optionally, you could use a reverse
proxy like nginx to support https or other stuff.

## Safe shutdown

Please send a SIGINT to the process to let it shut down
gracefully. Failure in doing so may result in data
corruption, as cache is not flushed to disk.
