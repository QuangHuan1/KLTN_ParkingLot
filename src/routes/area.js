const express = require('express');
const router = express.Router();
const areaController = require('../app/controllers/area-controller');

router.get('/:slug', areaController.showCars);
router.get('/', areaController.index);

module.exports = router;
