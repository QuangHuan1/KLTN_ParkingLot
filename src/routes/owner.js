const express = require('express');
const router = express.Router();
const ownerController = require('../app/controllers/owner-controller');
const authMiddleware = require('../app/controllers/auth-middleware');

router.post('/store', authMiddleware.admin, ownerController.storeOwner);
router.put('/update/:slug', authMiddleware.admin, ownerController.updateOwner);
router.delete('/delete/:id', authMiddleware.admin, ownerController.deleteOwner);
router.get('/create', ownerController.createOwner);
router.get('/data/:slug', ownerController.getDataOwner);
router.get('/data', ownerController.getDataOwners);
router.get('/:slug', ownerController.getOwner);
router.get('/car/:slug', ownerController.getCarsOfOwner);
router.get('/', ownerController.index);

module.exports = router;
