let entry;

if (process.env.DEBUG) {
    entry = require('./build/Debug/ClipboardData.node');
} else {
    entry = require('./build/Release/ClipboardData.node');
}

module.exports = entry;