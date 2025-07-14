static validateEntityData(searchBy, lob) {
        return new Promise(async (resolve) => {
            try {
                let searchPayload = {};
                
                if (searchBy.eci && searchBy.eci.trim() !== '') {
                    searchPayload.eci = searchBy.eci.trim();
                }
                
                if (searchBy.spn && searchBy.spn.trim() !== '') {
                    searchPayload.spn = searchBy.spn.trim();
                }
                
                if (searchBy.ucn && searchBy.ucn.trim() !== '') {
                    searchPayload.ucn = searchBy.ucn.trim();
                }

                let res = await post(`rest/api/ref-data/search`, searchPayload);
                
                if (res && res.data && Array.isArray(res.data)) {
                    resolve({success: true, payload: res.data, reason: null});
                } else if (res && res.success) {
                    const {success, payload, reason} = res;
                    resolve({success, payload, reason});
                } else {
                    resolve({success: false, payload: null, reason: 'No data found'});
                }
            } catch (err) {
                console.error('Entity validation API error:', err);
                resolve({success: false, status: err.status, reason: err.statusMessage || 'Entity validation failed'});
            }
        });
    }
