FROM node:9.3-alpine

WORKDIR /usr/src/node-redis-addon
COPY . .

RUN apk add --no-cache make gcc g++ python && \
  npm install --unsafe-perm && \
  apk del make gcc g++ python

