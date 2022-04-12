const QPDF = require(__dirname + '/build/Release/qpdf.node');

const stream = require('stream');
// const util = require('util');

class Overlay extends stream.Transform {

    constructor(overlayData) {
        super();
        this._options = {
            overlayData
        };
        this._bufs = [];
    }
    
    _transform(chunk, encoding, done) {
        this._bufs.push(chunk);
        done();
    }

    _flush(done) {
        this._options.inData = Buffer.concat(this._bufs);
        const self = this;
        QPDF.overlay(this._options, (err, data) => {
            if (err) {
                return done(err);
            }
            self.push(data);
            done();
        })
    }

}

module.exports = Overlay;