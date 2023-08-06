const dashboardRouter = require('./dashboard');
const carInfoRouter = require('./car-info');
const ownerRouter = require('./owner');
const checkInRouter = require('./check-in');
const checkOutRouter = require('./check-out');
const billRouter = require('./bill');
const areaRouter = require('./area');
const gateRouter = require('./gate');
const deviceRouter = require('./device');
const confirmCheckinRouter = require('./confirm-check-in');
const loginRouter = require('./login');
const phoneRouter = require('./phone');
const alarmRouter = require('./alarm');
const checkInOutImageRouter = require('./check-in-out-image');

function route(app) {
    app.use('/check-in-out-image', checkInOutImageRouter);
    app.use('/check-out', checkOutRouter);
    app.use('/check-in', checkInRouter);
    app.use('/bill', billRouter);
    app.use('/owner', ownerRouter);
    app.use('/area', areaRouter);
    app.use('/gate', gateRouter);
    app.use('/device', deviceRouter);
    app.use('/confirm-check-in', confirmCheckinRouter);
    app.use('/car-info', carInfoRouter);
    app.use('/alarm', alarmRouter);
    app.use('/phone', phoneRouter);
    app.use('/login', loginRouter);
    app.use('/', dashboardRouter);
}

module.exports = route;
