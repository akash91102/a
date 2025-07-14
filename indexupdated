import moment from "moment";
import React, { Component } from "react";
import autoBind from "react-autobind";
import { connect } from "react-redux";
import { bindActionCreators } from "redux";
import { 
    configSetup, 
    newFormBind, 
    tableEntryUpdate,
    formBind 
} from "../../../../../../javascript/config";
import { fetchConfig } from "./config/config";
import API from '../../../api';

import {
    objectExists,
    traverseFormToBindValue,
    cloneObject,
    traverseObjectAndReturn,
    traverseObject,
    traverseObjectAndInsert,
    updateObjectWithMatchingKeys,
    Debouncer
} from "../../../../../../javascript/utility";

import { mixinCheckGlobalConditions, mixinIsFormValid, mixinToggleModal } from "../../../../../../javascript/mixins";

import { Spinner, Dialog, DialogContent, DialogActions, Button, Icon } from "@jpmuitk/components";
import { DataGrid } from "@jpmuitk/data-grid";

import Buttons from "../../../../../Common/Buttons/Buttons";
import FormBuilder from "../../../../../Common/FormBuilder";
import Header from "../../../../../Common/Header/Header";
import AddOnProduct from "../../../../../Common/AddOnProduct/AddOnProduct";

import * as appStateAction from "../../../../../../actions/appStateAction";
import { isReadWrite } from "../../../../../../javascript/entitlementUtils";
import { MessageBox } from "../../../../../Common/messagebox";

const debounce = new Debouncer();
let selectedRowIdx = null;
let editingRowData = null;
let isEditMode = false;
let filteredData = null;

class XMProfileFields extends Component {
    constructor(props) {
        super(props);

        this.gridApiRefs = {};

        this.state = {
            permissions: {
                readonly: this.getPermission(props),
            },
            config: null,
            toggleState: true,
            ready: true,
            selectedRows: [], 
            modal: {
                disabled: null,
                show: false,
                header: {},
                formdata: null,
                onUpdate: () => { },
                onSave: () => { },
            },
            resetFormFields: {
                resetIfSelected: [{ field: "frequency", value: "*" }],
                resetExclude: ["frequency"],
            },
        };

        this.bindedEvents = {
            CREATE_NEW_MASTER_AGREEMENT: () => {
                this.openMasterAgreementModal(false);
            },
            EDIT_MASTER_AGREEMENT: () => {
                this.editSelectedMasterAgreement();
            },
            DELETE_SELECTED_MASTER_AGREEMENTS: () => {
                this.deleteSelectedRows();
            },
            VALIDATE_ECI_ON_BLUR: () => {
                this.validateEciOnBlur(); // Modal form ECI validation
            },
            VALIDATE_SPN_ON_BLUR: () => {
                this.validateSpnOnBlur(); // Agreement form SPN validation  
            },
            VALIDATE_ECID_ON_BLUR: () => {
                this.validateEcidOnBlur(); // Agreement form ECID validation
            },
            VALIDATE_TIER_ON_BLUR: () => {
                this.validateTierOnBlur();
            },
            VALIDATE_HOUSE_MULTIPLIER_ON_BLUR: () => {
                this.validateHouseMultiplierOnBlur();
            },
            HANDLE_MASTER_AGREEMENT_UPDATE: (agreementData) => {
                this.handleMasterAgreementUpdate(agreementData);
            }
        };

        autoBind(this);

        this.buttons = this.state.permissions.readonly
            ? [
                {
                    title: "Back",
                    onclick: this.props.onBack || this.props.onCancel,
                },
                { title: "Cancel", onclick: this.props.onCancel },
            ]
            : [
                {
                    title: "Back",
                    onclick: this.props.onBack || this.props.onCancel,
                },
                { title: "Cancel", onclick: this.props.onCancel },
                {
                    title: "Save Draft",
                    onclick: this.props.onSave,
                    displayIf: this.isProfileNewOrActive.bind(this),
                },
                {
                    title: "Submit",
                    onclick: this.handleSubmit,
                    displayIf: this.isProfileNewOrActive.bind(this),
                },
                {
                    title: "Deactivate",
                    onclick: this.props.onDeactivate,
                    displayIf: this.isProfileActive.bind(this),
                },
                {
                    title: "Activate",
                    onclick: this.props.onActivate,
                    displayIf: this.isProfileInactive.bind(this),
                },
            ];
    }

    getCurrentMasterAgreements = () => {
        const data = this.props.data || {};
        const bind = 'compositeProfile.legalAgreement.attributes.masterAgreements';
        const result = traverseObjectAndReturn({obj: data, string: bind});
        
        if (result && result.value && Array.isArray(result.value)) {
            return result.value;
        } else if (Array.isArray(result)) {
            return result;
        }
        return [];
    };

    updateMasterAgreements = (newAgreements, callback = null) => {
        const agreementsArray = Array.isArray(newAgreements) ? newAgreements : [];
        
        let formData = cloneObject(this.props.data || {});
        this.initializeProfileData(formData);
        
        const prop = 'compositeProfile.legalAgreement.attributes.masterAgreements';
        
        if (traverseObjectAndReturn({obj: formData, string: prop}) === null) {
            traverseObject({obj: formData, string: prop, insertAtEnd: {
                source: 'USER',
                sourceId: this.props.userInfo?.username || 'system',
                value: []
            }});
        }

        let masterAgreementsObj = traverseObjectAndReturn({obj: formData, string: prop});
        if (!masterAgreementsObj) {
            masterAgreementsObj = {
                source: 'USER',
                sourceId: this.props.userInfo?.username || 'system',
                value: []
            };
        }
        
        masterAgreementsObj.value = [...agreementsArray];
        masterAgreementsObj.source = 'USER';
        masterAgreementsObj.sourceId = this.props.userInfo?.username || 'system';

        traverseObjectAndInsert({
            obj: formData, 
            string: prop, 
            bindOn: 'masterAgreements', 
            newValues: masterAgreementsObj
        });

        if (this.props.onCustFormUpdate) {
            this.props.onCustFormUpdate(formData);
        }

        if (this.gridApiRefs["masterAgreements"]) {
            this.gridApiRefs["masterAgreements"].setRowData([...agreementsArray]);
        }

        if (callback) callback();
    };

    editSelectedMasterAgreement = () => {
        const { selectedRows } = this.state;
        if (selectedRows.length === 0) {
            MessageBox.info('No Selection', 'Please select a row to edit.');
            return;
        }
        if (selectedRows.length > 1) {
            MessageBox.info('Multiple Selection', 'Please select only one row to edit.');
            return;
        }
        this.openMasterAgreementModal(true);
    };

    openMasterAgreementModal = (editMode = false) => {
        const data = this.props.data || {};
        let newEntry = null;

        selectedRowIdx = null;
        editingRowData = null;
        isEditMode = editMode;

        if (editMode) {
            const { selectedRows } = this.state;
            if (selectedRows.length === 0) {
                MessageBox.info('No Selection', 'Please select a row to edit.');
                return;
            }
            if (selectedRows.length > 1) {
                MessageBox.info('Multiple Selection', 'Please select only one row to edit.');
                return;
            }
            editingRowData = selectedRows[0];
        }

        let formData = newFormBind({ formdata: this.state.config.create.newMasterAgreements });

        if (editMode && editingRowData) {
            formData = this.prepopulateFormData(formData, editingRowData);
        }

        this.toggleModal({
            state: true,
            header: { title: editMode ? 'Edit Master Agreement' : 'Add New Master Agreement' },
            formdata: formData,
            onUpdate: (newdata) => {
                newEntry = tableEntryUpdate({ newdata, markSource: true });
            },
            onSave: async () => {
                this.setState({ resetTable: true }, async () => {
                    try {
                        const validationData = {
                            lineOfBusiness: newEntry.lineOfBusiness?.value || '',
                            agreementId: newEntry.agreementId?.value || '',
                            profileType: newEntry.agreementType?.value || '',
                            lei: newEntry.legalEntityId?.value || '', // Updated to use 'lei' as per API requirements
                            legalEntity: newEntry.legalEntity?.value || '',
                            eci: newEntry.eci?.value || ''
                        };

                        if (!validationData.agreementId || !validationData.lineOfBusiness || !validationData.lei) {
                            MessageBox.error('Validation Error', 'Please fill in all required fields before saving.', 500);
                            this.setState({ resetTable: false });
                            return;
                        }

                        // Removed API validation call as requested

                        let exists = false;
                        const currentAgreements = this.getCurrentMasterAgreements();

                        if (currentAgreements) {
                            currentAgreements.forEach((item) => {
                                if (item.agreementId === newEntry.agreementId?.value) {
                                    if (!isEditMode || (isEditMode && JSON.stringify(item) !== JSON.stringify(editingRowData))) {
                                        exists = true;
                                        MessageBox.error('Validation Error', 'Agreement ID already exists. Please enter a unique Agreement ID.', 500);
                                    }
                                }
                            });
                        }

                        if (!exists) {
                            let updatedAgreements = [...currentAgreements];

                            if (isEditMode) {
                                const updatedAgreement = {
                                    ...editingRowData,
                                    lineOfBusiness: newEntry.lineOfBusiness?.value || '',
                                    agreementId: newEntry.agreementId?.value || '',
                                    profileType: newEntry.agreementType?.value || '',
                                    legalEntityId: newEntry.legalEntityId?.value || '',
                                    legalEntity: newEntry.legalEntity?.value || '',
                                    eci: newEntry.eci?.value || '',
                                    updatedAt: moment().format("YYYY-MM-DD HH:mm:ss.SSS"),
                                    updatedBy: this.props.userInfo?.username || 'system'
                                };

                                const agreementIndex = updatedAgreements.findIndex(item =>
                                    JSON.stringify(item) === JSON.stringify(editingRowData)
                                );

                                if (agreementIndex !== -1) {
                                    updatedAgreements[agreementIndex] = updatedAgreement;
                                }

                                MessageBox.alert('Success', 'Master agreement has been updated successfully.', 500);
                            } else {
                                const newAgreement = {
                                    lineOfBusiness: newEntry.lineOfBusiness?.value || '',
                                    agreementId: newEntry.agreementId?.value || '',
                                    profileType: newEntry.agreementType?.value || '',
                                    legalEntityId: newEntry.legalEntityId?.value || '',
                                    legalEntity: newEntry.legalEntity?.value || '',
                                    eci: newEntry.eci?.value || '',
                                    status: newEntry.status?.value || 'LIVE',
                                    createdAt: moment().format("YYYY-MM-DD HH:mm:ss.SSS"),
                                    updatedAt: moment().format("YYYY-MM-DD HH:mm:ss.SSS"),
                                    createdBy: this.props.userInfo?.username || 'system',
                                    updatedBy: this.props.userInfo?.username || 'system'
                                };

                                updatedAgreements.push(newAgreement);
                                MessageBox.alert('Success', 'Master agreement has been added successfully.', 500);
                            }

                            this.toggleModal({ state: false });

                            this.updateMasterAgreements(updatedAgreements, () => {
                                this.setState({ resetTable: false, selectedRows: [] });
                                this.rebuildCrossMarginEntities(updatedAgreements);
                            });
                        } else {
                            this.setState({ resetTable: false });
                        }

                    } catch (error) {
                        MessageBox.error('System Error', 'An unexpected error occurred. Please try again.', 500);
                        this.setState({
                            resetTable: false,
                            modal: { ...this.state.modal, disabled: false }
                        });
                    }
                });
            }
        });
    };

    prepopulateFormData = (formData, rowData) => {
        let populatedFormData = cloneObject(formData);

        if (populatedFormData.data && populatedFormData.data[0] && populatedFormData.data[0].column) {
            populatedFormData.data[0].column.forEach(field => {
                switch (field.bind) {
                    case 'lineOfBusiness':
                        field.value = rowData.lineOfBusiness || '';
                        break;
                    case 'agreementId':
                        field.value = rowData.agreementId || '';
                        break;
                    case 'agreementType':
                        field.value = rowData.profileType || '';
                        break;
                    case 'legalEntityId':
                        field.value = rowData.legalEntityId || '';
                        break;
                    case 'legalEntity':
                        field.value = rowData.legalEntity || '';
                        break;
                    case 'eci':
                        field.value = rowData.eci || '';
                        break;
                    default:
                        break;
                }
            });
        }

        return populatedFormData;
    };

    rebuildCrossMarginEntities = (masterAgreements) => {
        
        let formData = cloneObject(this.props.data || {});
        const crossMarginEntitiesPath = 'compositeProfile.legalAgreement.attributes.crossMarginEntities';
        
        let crossMarginEntities = traverseObjectAndReturn({obj: formData, string: crossMarginEntitiesPath});
        if (!crossMarginEntities) {
            crossMarginEntities = {
                source: 'USER',
                sourceId: this.props.userInfo?.username || 'system',
                value: []
            };
        }

        const validAgreements = Array.isArray(masterAgreements) ? masterAgreements : [];

        if (validAgreements.length === 0) {
            crossMarginEntities.value = [];
        } else {
            const groupedAgreements = {};
            validAgreements.forEach(agreement => {
                const entityKey = `${agreement.eci || 'unknown'}_${agreement.legalEntityId || 'unknown'}`;
                if (!groupedAgreements[entityKey]) {
                    groupedAgreements[entityKey] = {
                        eci: agreement.eci || '',
                        legalEntityId: agreement.legalEntityId || '',
                        legalEntity: agreement.legalEntity || '',
                        ucn: agreement.ucn || '',
                        spn: agreement.spn || '',
                        agreements: {}
                    };
                }
                
                const lobKey = agreement.lineOfBusiness || 'UNKNOWN';
                if (!groupedAgreements[entityKey].agreements[lobKey]) {
                    groupedAgreements[entityKey].agreements[lobKey] = [];
                }
                
                groupedAgreements[entityKey].agreements[lobKey].push({
                    profileId: agreement.agreementId,
                    lineOfBusiness: agreement.lineOfBusiness,
                    profileType: agreement.profileType,
                    profileStatus: agreement.status,
                    version: agreement.version || 1,
                    createdBy: agreement.createdBy,
                    updatedBy: agreement.updatedBy,
                    createdDtm: agreement.createdAt,
                    updatedDtm: agreement.updatedAt,
                    attributes: null,
                    approvedDtm: null,
                    approvedBy: null,
                    comments: null,
                    activationDtm: null
                });
            });

            crossMarginEntities.value = Object.values(groupedAgreements).map(group => ({
                counterParty: {
                    role: "COUNTERPARTY",
                    eci: group.eci,
                    ucn: group.ucn,
                    spn: group.spn,
                    name: group.legalEntity,
                    lei: null,
                    sapLeCode: null,
                    principalLei: null,
                    principalEci: null,
                    cbd: null,
                    leCode: null,
                    goldenEntityEci: null,
                    goldenEntityName: null
                },
                legalEntities: [{
                    legalEntity: {
                        role: "LEGAL_OWNER",
                        eci: group.eci,
                        ucn: group.ucn,
                        spn: group.spn,
                        name: group.legalEntity,
                        lei: null,
                        sapLeCode: null,
                        principalLei: null,
                        principalEci: null,
                        cbd: null,
                        leCode: null,
                        goldenEntityEci: null,
                        goldenEntityName: null
                    },
                    legalAgreementProfiles: group.agreements
                }]
            }));
        }

        crossMarginEntities.source = 'USER';
        crossMarginEntities.sourceId = this.props.userInfo?.username || 'system';

        traverseObjectAndInsert({
            obj: formData,
            string: crossMarginEntitiesPath,
            bindOn: 'crossMarginEntities',
            newValues: crossMarginEntities
        });

        if (this.props.onCustFormUpdate) {
            this.props.onCustFormUpdate(formData);
        }
    };

    handleMasterAgreementUpdate = (agreementData) => {
        debounce.add(null, async () => {
            if (!agreementData || !agreementData.agreementId) {
                return;
            }

            try {
                const validationResult = await API.validateMasterAgreement(
                    agreementData, 
                    this.props.appState?.lob?.toUpperCase() || 'XM'
                );

                if (validationResult.success) {
                    const currentAgreements = this.getCurrentMasterAgreements();
                    const updatedAgreements = currentAgreements.map(agreement => 
                        agreement.agreementId === agreementData.agreementId ? agreementData : agreement
                    );
                    
                    this.updateMasterAgreements(updatedAgreements);
                } else {
                    MessageBox.error('Validation Failed', validationResult.reason || 'Agreement validation failed.', 500);
                }
            } catch (error) {
                console.warn('Master agreement update validation failed:', error);
            }
        }, 600);
    };

    getPermission(props) {
        const profileReadOnly = props.data?.profileReadOnly || false;
        const userHasWriteAccess = isReadWrite(props.userInfo, "ROLE_AGENCY_UI_READ_WRITE");
        return profileReadOnly || !userHasWriteAccess;
    }

    isProfileNewOrActive() {
        const data = this.props.data || {};
        const profilePresent = objectExists(data, "compositeProfile.legalAgreement.profileStatus");
        const status = data.compositeProfile?.legalAgreement?.profileStatus;
        return !profilePresent || status === "LIVE" || status === "NEW";
    }

    isProfileActive() {
        const data = this.props.data || {};
        const profilePresent = objectExists(data, "compositeProfile.legalAgreement.profileStatus");
        const status = data.compositeProfile?.legalAgreement?.profileStatus;
        return profilePresent && status === "LIVE";
    }

    isProfileInactive() {
        const data = this.props.data || {};
        const profilePresent = objectExists(data, "compositeProfile.legalAgreement.profileStatus");
        const status = data.compositeProfile?.legalAgreement?.profileStatus;
        return profilePresent && status === "INACTIVE";
    }

    componentDidMount() {
        try {
            const config = fetchConfig(this.props.appState?.lob || "xm");
            this.setup(config);
            
           
        } catch (error) {
            console.error('Failed to fetch config:', error);
            const fallbackConfig = {
                layout: [
                    {
                        refKey: "agreement",
                        header: { title: "XM Profile Information" },
                        type: "form",
                        data: [],
                    },
                ],
                create: {
                    newMasterAgreements: {
                        data: [{ column: [] }]
                    }
                }
            };
            this.setup(fallbackConfig);
        }
        
        this.setState({
            hasLocalChanges: false,
            lastUpdateTimestamp: null
        });
    }

    componentDidUpdate(prevProps) {
        if (prevProps.data !== this.props.data && this.props.data) {
            const newMasterAgreements = this.getCurrentMasterAgreements();
            const validNewAgreements = Array.isArray(newMasterAgreements) ? newMasterAgreements : [];
            
            const currentStateAgreements = Array.isArray(this.state.masterAgreements) ? this.state.masterAgreements : [];
            
            // Only update if there are actual changes and we're not in the middle of a local operation
            if (!this.state.hasLocalChanges && JSON.stringify(validNewAgreements) !== JSON.stringify(currentStateAgreements)) {
                this.setState({
                    masterAgreements: [...validNewAgreements],
                    hasLocalChanges: false
                });
                
                if (this.gridApiRefs["masterAgreements"]) {
                    this.gridApiRefs["masterAgreements"].setRowData([...validNewAgreements]);
                }
            }
        }
    }

    setup = (config) => {
        let cloneData = cloneObject(this.props.data || {});
        this.initializeProfileData(cloneData);

        this.processAndFlattenData(cloneData);

        let configSetupVar = configSetup({
            config,
            formdata: cloneData,
            staticData: this.props.staticData,
            bindFunctions: this.bindedEvents,
        });

        this.setState({ config: configSetupVar });

        if (this.props.onCustFormUpdate) {
            this.props.onCustFormUpdate(cloneData);
        }
    };

    processAndFlattenData = (data) => {
        if (objectExists(data, 'compositeProfile.legalAgreement.attributes.crossMarginEntities.value')) {
            const crossMarginEntities = data.compositeProfile.legalAgreement.attributes.crossMarginEntities.value;
            
            if (crossMarginEntities.length > 0) {
                const firstEntity = crossMarginEntities[0];
                
                if (firstEntity.counterParty) {
                    traverseObjectAndInsert({
                        obj: data,
                        string: 'compositeProfile.legalAgreement.attributes.eci',
                        bindOn: 'eci',
                        newValues: firstEntity.counterParty.eci
                    });
                    
                    traverseObjectAndInsert({
                        obj: data,
                        string: 'compositeProfile.legalAgreement.attributes.spn',
                        bindOn: 'spn',
                        newValues: firstEntity.counterParty.spn
                    });
                }

                const flattenedAgreements = this.processLegalAgreementProfiles(data);
         
                traverseObjectAndInsert({
                    obj: data,
                    string: 'compositeProfile.legalAgreement.attributes.masterAgreements',
                    bindOn: 'masterAgreements',
                    newValues: {
                        source: 'USER',
                        sourceId: this.props.userInfo?.username || 'system',
                        value: flattenedAgreements
                    }
                });
            }
        }
    };

    initializeProfileData = (data) => {
        if (!data.compositeProfile) {
            data.compositeProfile = {};
        }

        if (!data.compositeProfile.legalAgreement) {
            data.compositeProfile.legalAgreement = {
                profileId: "",
                profileType: "",
                version: 0,
                updatedBy: "",
                updatedDtm: moment().format("YYYY-MM-DD HH:mm:ss.SSS"),
                attributes: {},
            };
        }

        if (!data.compositeProfile.legalAgreement.attributes) {
            data.compositeProfile.legalAgreement.attributes = {
                agreementType: "",
                spn: "",
                eci: "",
                tier: "",
                tierVersion: "",
                tierEffectiveDate: "",
                addOn: '',
                houseMultiplier: "",
                status: "NEW",
            };
        }

        if (typeof data.compositeProfile.legalAgreement.version === "number") {
            data.compositeProfile.legalAgreement.version = data.compositeProfile.legalAgreement.version.toString();
        }

        if (!data.compositeProfile.legalAgreement.attributes.masterAgreements) {
            data.compositeProfile.legalAgreement.attributes.masterAgreements = {
                source: 'USER',
                sourceId: this.props.userInfo?.username || 'system',
                value: []
            };
        }
    };

    onGridReady = ({ e, refKey }) => {
        const { api } = e;
        this.gridApiRefs[refKey] = api;

        setTimeout(() => {
            api.sizeColumnsToFit();
        }, 100);

        if (refKey === "masterAgreements") {
            api.addEventListener('selectionChanged', this.onSelectionChanged);
            // Removed double-click edit functionality as requested
            
            const masterAgreements = this.getCurrentMasterAgreements();
            const validMasterAgreements = Array.isArray(masterAgreements) ? masterAgreements : [];
            if (validMasterAgreements.length > 0) {
                api.setRowData([...validMasterAgreements]);
            }
        }
    };

    onSelectionChanged = () => {
        const selectedNodes = this.gridApiRefs["masterAgreements"]?.getSelectedNodes() || [];
        const selectedRows = selectedNodes.map(node => node.data);
        this.setState({ selectedRows });
    };

    onCellValueChanged = (event) => {
        const { data, colDef, newValue, node } = event;
        const field = colDef.field;

        data[field] = newValue;
        data.updatedAt = moment().format("YYYY-MM-DD HH:mm:ss.SSS");
        data.updatedBy = this.props.userInfo?.username || 'system';
        
        const currentAgreements = this.getCurrentMasterAgreements();
        const rowIndex = node.rowIndex;
        
        if (currentAgreements[rowIndex]) {
            currentAgreements[rowIndex] = data;
            this.updateMasterAgreements(currentAgreements);
        }
    };

    deleteSelectedRows = () => {
        const { selectedRows } = this.state;

        if (selectedRows.length === 0) {
            MessageBox.info('No Selection', 'Please select one or more rows to delete.');
            return;
        }

        const currentAgreements = this.getCurrentMasterAgreements();
        const validCurrentAgreements = Array.isArray(currentAgreements) ? currentAgreements : [];
        
        const updatedAgreements = validCurrentAgreements.filter(agreement =>
            !selectedRows.some(selectedRow =>
                JSON.stringify(agreement) === JSON.stringify(selectedRow)
            )
        );

        // Set local changes flag to prevent componentDidUpdate from overriding our changes
        this.setState({ hasLocalChanges: true });

        this.updateMasterAgreements(updatedAgreements, () => {
            this.setState({ 
                selectedRows: [],
                hasLocalChanges: true // Maintain the flag after update
            });
            this.rebuildCrossMarginEntities(updatedAgreements);
        });
    };

    // New validation methods for SPN and ECID in agreement form
    validateSpnOnBlur = async () => {
        const { config } = this.state;
        if (!config) return;

        let spnValue = null;
        let updatedConfig = cloneObject(config);
        let hasUpdated = false;

        // Find SPN value in the agreement form
        updatedConfig.layout.find(x => x.refKey === 'agreement').data.forEach(section => {
            section.column.forEach(field => {
                if (field.title === 'SPN' && field.value) {
                    spnValue = field.value.toString().trim();
                }
            });
        });

        if (spnValue && spnValue.length === 7) {
            try {
                const validationResult = await API.validateEntityData(
                    { spn: spnValue }, 
                    this.props.appState?.lob?.toUpperCase() || 'XM'
                );

                if (validationResult.success && validationResult.payload && Array.isArray(validationResult.payload) && validationResult.payload.length > 0) {
                    const entityData = validationResult.payload[0];
                    
                    // Update ECID field with API response
                    updatedConfig.layout.find(x => x.refKey === 'agreement').data.forEach(section => {
                        section.column.forEach(field => {
                            if (field.title === 'ECID') {
                                field.value = entityData.eci || '';
                                hasUpdated = true;
                            }
                        });
                    });

                    if (hasUpdated) {
                        this.setState({ config: updatedConfig }, () => {
                            this.handleFormDataUpdate(updatedConfig.layout.find(x => x.refKey === 'agreement').data, 'agreement');
                        });
                    }
                } else {
                    MessageBox.error('Validation Error', 'Invalid SPN. No matching entity found.', 500);
                }
            } catch (error) {
                console.warn('SPN validation failed:', error);
                MessageBox.error('Validation Error', 'Unable to validate SPN. Please check the value and try again.', 500);
            }
        } else if (spnValue && spnValue.length !== 7) {
            MessageBox.error('Validation Error', 'SPN must be exactly 7 characters long.', 500);
        }
    };

    validateEcidOnBlur = async () => {
        const { config } = this.state;
        if (!config) return;

        let ecidValue = null;
        let updatedConfig = cloneObject(config);
        let hasUpdated = false;

        // Find ECID value in the agreement form
        updatedConfig.layout.find(x => x.refKey === 'agreement').data.forEach(section => {
            section.column.forEach(field => {
                if (field.title === 'ECID' && field.value) {
                    ecidValue = field.value.toString().trim();
                }
            });
        });

        if (ecidValue && ecidValue.length === 10) {
            try {
                const validationResult = await API.validateEntityData(
                    { eci: ecidValue }, 
                    this.props.appState?.lob?.toUpperCase() || 'XM'
                );

                if (validationResult.success && validationResult.payload && Array.isArray(validationResult.payload) && validationResult.payload.length > 0) {
                    const entityData = validationResult.payload[0];
                    
                    // Update SPN field with API response
                    updatedConfig.layout.find(x => x.refKey === 'agreement').data.forEach(section => {
                        section.column.forEach(field => {
                            if (field.title === 'SPN') {
                                field.value = entityData.spn || '';
                                hasUpdated = true;
                            }
                        });
                    });

                    if (hasUpdated) {
                        this.setState({ config: updatedConfig }, () => {
                            this.handleFormDataUpdate(updatedConfig.layout.find(x => x.refKey === 'agreement').data, 'agreement');
                        });
                    }
                } else {
                    MessageBox.error('Validation Error', 'Invalid ECID. No matching entity found.', 500);
                }
            } catch (error) {
                console.warn('ECID validation failed:', error);
                MessageBox.error('Validation Error', 'Unable to validate ECID. Please check the value and try again.', 500);
            }
        } else if (ecidValue && ecidValue.length !== 10) {
            MessageBox.error('Validation Error', 'ECID must be exactly 10 characters long.', 500);
        }
    };

    validateStringFieldOnBlur = (fieldTitle, requiredLength, errorMessage) => {
        let clone = cloneObject(this.state.config);
        let hasFieldChanges = false;
        let updatedFieldBinding = null;
        let updatedFieldValue = null;
        
        clone.layout.find(x => x.refKey === 'agreement').data.forEach(x => {
            x.column.forEach(form => {
                if (form.title === fieldTitle && form.value !== undefined) {
                    if (form.value === "" || form.value === null || typeof form.value === "undefined") {
                        return;
                    }
                    
                    let currentValue = form.value.toString();
                    if (currentValue.length !== requiredLength) {
                        hasFieldChanges = true;
                        updatedFieldBinding = form.bind;
                        updatedFieldValue = currentValue;
                        MessageBox.error('Validation Error', errorMessage, 500);
                    }
                }
            });
        });
   
        if (hasFieldChanges) {
            this.setState({ config: clone, ready: false }, () => {
                this.setState({ ready: true });
            });
        }
    };

    validateEciOnBlur = async () => {
        const { config } = this.state;
        if (!config) return;

        let eciValue = null;
        let updatedConfig = cloneObject(config);
        let hasUpdated = false;

        // Find the ECI field in the modal form data
        if (this.state.modal.formdata && this.state.modal.formdata.data) {
            this.state.modal.formdata.data.forEach(section => {
                if (section.column) {
                    section.column.forEach(field => {
                        if (field.bind === 'eci' && field.value) {
                            eciValue = field.value.toString().trim();
                        }
                    });
                }
            });
        }

        if (eciValue && eciValue.length > 0) {
            try {
                const validationResult = await API.validateEntityData(
                    { eci: eciValue }, 
                    this.props.appState?.lob?.toUpperCase() || 'XM'
                );

                if (validationResult.success && validationResult.payload && Array.isArray(validationResult.payload) && validationResult.payload.length > 0) {
                    const entityData = validationResult.payload[0];
                    
                    // Update modal form data with API response
                    let updatedFormData = cloneObject(this.state.modal.formdata);
                    updatedFormData.data.forEach(section => {
                        if (section.column) {
                            section.column.forEach(field => {
                                switch (field.bind) {
                                    case 'legalEntityId':
                                        field.value = entityData.lei || entityData.ucn || '';
                                        hasUpdated = true;
                                        break;
                                    case 'legalEntity':
                                        field.value = entityData.name || '';
                                        hasUpdated = true;
                                        break;
                                }
                            });
                        }
                    });

                    if (hasUpdated) {
                        this.setState({
                            modal: {
                                ...this.state.modal,
                                formdata: updatedFormData
                            }
                        });
                    }
                } else {
                    MessageBox.error('Validation Error', 'Invalid ECID. No matching entity found.', 500);
                }
            } catch (error) {
                console.warn('ECI validation failed:', error);
                MessageBox.error('Validation Error', 'Unable to validate ECID. Please check the value and try again.', 500);
            }
        }
    };

    validateHouseMultiplierOnBlur = () => {
        this.validateFieldOnBlur(
            'House Multiplier', 
            0, 
            100, 
            'House Multiplier value cannot be less than 0. It has been reset to 0.', 
            'House Multiplier value cannot be greater than 100. It has been reset to 100.'
        );
    };

    validateTierOnBlur = () => {
        this.validateFieldOnBlur(
            'Tier', 
            1, 
            9, 
            'Tier value cannot be less than 1. It has been reset to 1.', 
            'Tier value cannot be greater than 9. It has been reset to 9.'
        );
    };

    validateFieldOnBlur = (fieldTitle, minValue, maxValue, minErrorMessage, maxErrorMessage) => {
        let clone = cloneObject(this.state.config);
        let hasFieldChanges = false;
        let updatedFieldBinding = null;
        let updatedFieldValue = null;
        
        clone.layout.find(x => x.refKey === 'crossMarginEntities').data.forEach(x => {
            x.column.forEach(form => {
                if (form.title === fieldTitle && form.value !== undefined) {
                    if (form.value === "" || form.value === null || typeof form.value === "undefined") {
                        return;
                    }
                    
                    let newValue = form.value;
                    if (newValue < minValue) {
                        newValue = minValue;
                        form.value = newValue;
                        hasFieldChanges = true;
                        updatedFieldBinding = form.bind;
                        updatedFieldValue = newValue;
                        MessageBox.error('Validation Error', minErrorMessage, 500);
                    } else if (newValue > maxValue) {
                        newValue = maxValue;
                        form.value = newValue;
                        hasFieldChanges = true;
                        updatedFieldBinding = form.bind;
                        updatedFieldValue = newValue;
                        MessageBox.error('Validation Error', maxErrorMessage, 500);
                    }
                }
            });
        });
   
        if (hasFieldChanges) {
            this.setState({ config: clone, ready: false }, () => {
                this.setState({ ready: true });
       
                if (this.props.onCustFormUpdate && updatedFieldBinding && updatedFieldValue !== null) {
                    let formData = cloneObject(this.props.data || {});
                    this.initializeProfileData(formData);
                    
                    this.updateFieldInFormData(formData, updatedFieldBinding, updatedFieldValue, 'USER');
                    
                    this.props.onCustFormUpdate(formData);
                }
            });
        }
    };

    isFormValid = (props) => {
        mixinIsFormValid({ ...props, _this: this });
    };

    toggleModal = (props) => {
        mixinToggleModal({ ...props, _this: this });
    };

    checkGlobalConditions = (renderIf = null) => {
        return mixinCheckGlobalConditions({ renderIf, _this: this });
    };

    handleSubmit = () => {
        if (this.props.onSubmit) {
            this.props.onSubmit();
        }
    };

    handleFormDataUpdate = (updatedFormData, sectionRefKey = null) => {
        let completeFormData = cloneObject(this.props.data || {});
        let hasStructuralChanges = false;
        
        if (Array.isArray(updatedFormData)) {
            updatedFormData.forEach(section => {
                if (section.column && Array.isArray(section.column)) {
                    section.column.forEach(field => {
                        if (field.bind && field.value !== undefined) {
                            this.updateFieldInFormData(completeFormData, field.bind, field.value, field.source);
                            
                            if (this.isStructuralField(field.bind)) {
                                hasStructuralChanges = true;
                            }
                        }
                    });
                }
            });
        }
        
        this.initializeProfileData(completeFormData);
        
        if (hasStructuralChanges && sectionRefKey !== 'crossMarginEntities') {
            const currentAgreements = this.getCurrentMasterAgreements();
            if (Array.isArray(currentAgreements) && currentAgreements.length > 0) {
                this.rebuildCrossMarginEntities(currentAgreements);
            }
        }
        
        if (this.props.onCustFormUpdate) {
            this.props.onCustFormUpdate(completeFormData);
        }

        this.setState({ 
            hasLocalChanges: true,
            lastUpdateTimestamp: Date.now() 
        });
    };

    isStructuralField = (fieldBinding) => {
    
        const structuralFields = [
            'compositeProfile.legalAgreement.attributes.eci',
            'compositeProfile.legalAgreement.attributes.spn',
            'compositeProfile.legalAgreement.attributes.counterPartyUcn',
            'compositeProfile.legalAgreement.attributes.legalEntityUcn',
            'compositeProfile.legalAgreement.attributes.parentLegalEntityUcn',
            'compositeProfile.legalAgreement.attributes.parties'
        ];
       
        const nonStructuralFields = [
            'compositeProfile.legalAgreement.attributes.tier',
            'compositeProfile.legalAgreement.attributes.tierVersion',
            'compositeProfile.legalAgreement.attributes.tierEffectiveDate',
            'compositeProfile.legalAgreement.attributes.houseMultiplier',
            'compositeProfile.legalAgreement.attributes.addOn'
        ];
        
        if (nonStructuralFields.some(field => fieldBinding.includes(field))) {
            return false;
        }
        
        return structuralFields.some(field => fieldBinding.includes(field));
    };

    updateFieldInFormData = (formData, fieldBinding, fieldValue, fieldSource = 'USER') => {
        const pathParts = fieldBinding.split('.');
        let currentObject = formData;
        
        for (let i = 0; i < pathParts.length - 1; i++) {
            if (!currentObject[pathParts[i]]) {
                currentObject[pathParts[i]] = {};
            }
            currentObject = currentObject[pathParts[i]];
        }
        
        const finalKey = pathParts[pathParts.length - 1];
        
        const requiresMetadata = this.shouldUseMetadataStructure(fieldBinding);
        
        if (requiresMetadata && fieldSource && fieldSource !== 'SYSTEM') {
            currentObject[finalKey] = {
                source: fieldSource,
                sourceId: this.props.userInfo?.username || 'system',
                value: fieldValue
            };
        } else {
            currentObject[finalKey] = fieldValue;
        }
    };

    shouldUseMetadataStructure = (fieldBinding) => {
        const metadataFields = [
            'compositeProfile.legalAgreement.attributes.tier',
            'compositeProfile.legalAgreement.attributes.tierVersion',
            'compositeProfile.legalAgreement.attributes.tierEffectiveDate',
            'compositeProfile.legalAgreement.attributes.houseMultiplier',
            'compositeProfile.legalAgreement.attributes.addOn',
            'compositeProfile.legalAgreement.attributes.spn',
            'compositeProfile.legalAgreement.attributes.eci',
            'compositeProfile.legalAgreement.profileStatus',
            'compositeProfile.legalAgreement.version'
        ];
        
        return metadataFields.some(field => fieldBinding.includes(field));
    };

    render() {
        const {
            buttons,
            bindedEvents,
            checkGlobalConditions,
            toggleModal,
            onGridReady,
            isFormValid,
        } = this;

        const { ready, modal, config, permissions } = this.state;
        const { onFormUpdate, onCustFormUpdate, staticData } = this.props;

        let formData = this.props.data || {};

        return (
            <div style={{ marginTop: "10px", marginBottom: "10px" }}>
                {!!config && ready ? (
                    <>
                        {config.layout.map((x, i) => {
                            const {
                                refKey,
                                header,
                                type,
                                data,
                                columnDefs,
                                renderIf,
                                bind,
                                rowClassRules,
                                createKey,
                                markSource
                            } = x;

                            return checkGlobalConditions(renderIf) ? (
                                <div className="dt-container" key={i}>
                                    {!!header ? (
                                        <Header
                                            {...header}
                                            bindedEvents={bindedEvents}
                                            permissions={permissions}
                                        />
                                    ) : null}

                                    {!!type && type === "form" ? (
                                        <FormBuilder
                                            data={data}
                                            onFormUpdate={(updatedData) => {

                                                if (onFormUpdate) {
                                                    onFormUpdate(updatedData);
                                                }
                                                
                                                this.handleFormDataUpdate(updatedData, refKey);
                                            }}
                                            bindedEvents={{
                                                ...bindedEvents
                                            }}
                                            permissions={permissions}
                                            staticData={staticData}
                                            renderComponent={(type, bind, containerIndex, formIndex) => {
                                                if (type === 'component' && bind.includes('addOn')) {
                                                    return (
                                                        <AddOnProduct
                                                            bind={bind}
                                                            formData={formData}
                                                            onCustFormUpdate={onCustFormUpdate}
                                                            bindedEvents={bindedEvents}
                                                            config={this.state.config}
                                                        />
                                                    );
                                                }
                                                return null;
                                            }}
                                        />
                                    ) : null}

                                    {!!type && type === "datatable" ? (
                                        <DataGrid
                                            suppressRowClickSelection={false}
                                            rowSelection="multiple"
                                            rowMultiSelectWithClick={true}
                                            headerCheckboxSelection={true}
                                            headerCheckboxSelectionFilteredOnly={true}
                                            containerProps={{
                                                style: { height: "200px" },
                                            }}
                                            rowStripes
                                            columnDefs={columnDefs?.map(col => ({
                                                ...col,
                                                editable: false,
                                                checkboxSelection: col.field === "lineOfBusiness" ? true : false,
                                                headerCheckboxSelection: col.field === "lineOfBusiness" ? true : false,
                                            }))}
                                            rowData={traverseFormToBindValue({
                                                formData,
                                                bind,
                                                filterFunction: () => true
                                            })}
                                            onCellValueChanged={this.onCellValueChanged}
                                            onGridReady={(e) => {
                                                onGridReady({ e, refKey });
                                            }}
                                            refKey={refKey}
                                        />
                                    ) : null}
                                </div>
                            ) : null;
                        })}

                        <Dialog
                            onEntered={() => { }}
                            open={modal.show}
                            onClick={() => { }}
                            onClose={() => { toggleModal(false) }}
                        >
                            <div>
                                {!!modal.show ? (
                                    <DialogContent>
                                        <div className='dt-container'>
                                            <Header {...modal.header} permissions={permissions} />
                                            <FormBuilder
                                                data={this.state.modal.formdata?.data || []}
                                                onFormUpdate={modal.onUpdate}
                                                isFormValid={isFormValid}
                                                filteredData={filteredData}
                                                selectedRowIdx={selectedRowIdx}
                                                gridApi={this.gridApiRefs}
                                                resetFormFields={this.state.resetFormFields}
                                                permissions={permissions}
                                                staticData={staticData}
                                            />
                                        </div>
                                    </DialogContent>
                                ) : null}

                                <DialogActions>
                                    {!permissions.readonly ? (
                                        <Button variant={'cta'} disabled={modal.disabled} onClick={modal.onSave}>
                                            Save
                                        </Button>
                                    ) : null}
                                    <Button variant={'cta'} onClick={() => { toggleModal(false) }}>
                                        Close
                                    </Button>
                                </DialogActions>
                            </div>
                        </Dialog>

                        <Buttons data={buttons} />
                    </>
                ) : (
                    <div style={{ padding: "20px", textAlign: "center" }}>
                        <Spinner />
                        <p>Loading XM Profile configuration...</p>
                    </div>
                )}
            </div>
        );
    }

    processLegalAgreementProfiles = (data) => {
        let legalAgreementProfiles = [];
        const crossMarginEntities = data?.compositeProfile?.legalAgreement?.attributes?.crossMarginEntities?.value;

        if (Array.isArray(crossMarginEntities) && crossMarginEntities.length > 0) {
            crossMarginEntities.forEach((entityItem) => {
                const legalEntities = entityItem?.legalEntities;
                if (Array.isArray(legalEntities) && legalEntities.length > 0) {
                    legalEntities.forEach((entity) => {
                        const profilesObj = entity?.legalAgreementProfiles;
                        const legalEntity = entity?.legalEntity;
                        
                        if (profilesObj && typeof profilesObj === 'object') {
                            Object.keys(profilesObj).forEach(agreementType => {
                                const profiles = profilesObj[agreementType];
                                if (Array.isArray(profiles)) {
                                    profiles.forEach((profile) => {
                                        legalAgreementProfiles.push({
                                            agreementId: profile.profileId,
                                            lineOfBusiness: profile.lineOfBusiness,
                                            agreementCategory: agreementType,
                                            profileType: profile.profileType,
                                            legalEntityId: legalEntity?.ucn || '',
                                            legalEntity: legalEntity?.name || profile.lineOfBusiness,
                                            status: profile.profileStatus,
                                            updatedBy: profile.updatedBy,
                                            updatedAt: profile.updatedDtm,
                                            createdBy: profile.createdBy,
                                            createdAt: profile.createdDtm,
                                            version: profile.version,
                                            eci: legalEntity?.eci || '',
                                            _originalData: profile,
                                            _agreementType: agreementType
                                        });
                                    });
                                }
                            });
                        }
                    });
                }
            });
        }

        return legalAgreementProfiles;
    };
}

const mapStateToProps = (state) => {
    const { appState } = state;
    return { appState };
};

const mapDispatchToProps = (dispatch) => {
    return {
        appStateAction: bindActionCreators(appStateAction, dispatch),
    };
};

export default connect(mapStateToProps, mapDispatchToProps)(XMProfileFields);
