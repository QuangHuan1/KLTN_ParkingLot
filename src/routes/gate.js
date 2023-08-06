const express = require('express');
const router = express.Router();
const gateController = require('../app/controllers/gate-controller');
const authMiddleware = require('../app/controllers/auth-middleware');

router.post('/store', authMiddleware.tech, gateController.storeGate);
router.put('/update/:gateId', authMiddleware.tech, gateController.updateGate);
router.delete('/delete/:gateId', authMiddleware.tech, gateController.deleteGate);
router.get('/data/:gateCode', gateController.getDataGate);
router.get('/data', gateController.getDataGates);
router.get('/gate-tracking/:gateId', gateController.trackGate);
router.get('/', gateController.show);
module.exports = router;
