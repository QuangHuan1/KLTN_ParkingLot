const express = require('express');
const router = express.Router();
const areaController = require('../app/controllers/area-controller');
const authMiddleware = require('../app/controllers/auth-middleware');

router.post('/store', authMiddleware.tech, areaController.storeArea);
router.put('/update/:areaId', authMiddleware.tech, areaController.updateArea);
router.delete('/delete/:areaId', authMiddleware.tech, areaController.deleteArea);
router.get('/data/:id', areaController.getDataArea);
router.get('/data', areaController.getDataAreas);
router.get('/car/:areaCode', areaController.getCarsInArea);
router.get('/:areaCode', areaController.getArea);
router.get('/', areaController.index);

module.exports = router;
