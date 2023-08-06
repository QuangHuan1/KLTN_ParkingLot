const express = require('express');
const router = express.Router();
const dashboardController = require('../app/controllers/dashboard-controller');

router.get('/vacancy', dashboardController.getVacancies);
router.get('/num-registered', dashboardController.getNumRegisteredCar);
router.get('/', dashboardController.show);

module.exports = router;
