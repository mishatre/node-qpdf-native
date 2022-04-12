
FROM node:17-alpine3.14
WORKDIR /usr/src/app

COPY . .

RUN apk add --no-cache --virtual .gyp python3 pkgconf make g++ qpdf-dev \
    && npm install 
    # \
    # && apk del .gyp

ENTRYPOINT [ "npm", "run", "test" ]