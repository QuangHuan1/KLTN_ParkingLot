const siteRouter = require('./site');
const ownerRouter = require('./owner');
const checkInRouter = require('./check-in');
const checkOutRouter = require('./check-out');
const areaRouter = require('./area');

function route(app) {
    app.use('/check-out', checkOutRouter);
    app.use('/check-in', checkInRouter);
    app.use('/owner', ownerRouter);
    app.use('/area', areaRouter);
    app.use('/', siteRouter);
}

module.exports = route;
