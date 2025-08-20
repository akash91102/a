import React, { useState } from 'react';
import {
  SaltProvider,
  Button,
  Card,
  FlexLayout,
  StackLayout,
  Text,
  Input,
  FormField,
  FormFieldLabel,
  Dropdown,
  Option,
  Banner,
} from '@salt-ds/core';
import {
  CalendarIcon,
  FilterIcon,
} from '@salt-ds/icons';
import '@salt-ds/theme/index.css';
import './App.css';

const FinancialFilterForm = () => {
  const [formData, setFormData] = useState({
    region: 'AMERICA',
    csr: 'All',
    legalEntity: 'All',
    counterparty: 'All',
    cobFrom: '10-Oct-2022',
    cobTo: '11-Oct-2022',
    product: 'DERIVATIVES'
  });

  const [isLoading, setIsLoading] = useState(false);
  const [results, setResults] = useState(null);

  const regionOptions = ['AMERICA', 'EUROPE', 'INDIA', 'JANSA', 'SAMERICA', 'SAUDI'];
  
  const csrOptions = [
    'All',
    'AAMINAIZ, RAHAMAN',
    'ASYA REVA',
    'GURPANU BUYAK',
    'JESSICAGAIL BURNS',
    'JOHANNA PAOLA VELOZ',
    'JOSEPH HARBAUGH',
    'JOSHUAADAM MOSKEY',
    'KATE',
    'KEVIN LIN',
    'KIMBERLY WALDMAN',
    'KRISTERJANUS MCCONNELL',
    'LORENZGICHARES RUSSOMANNO'
  ];

  const legalEntityOptions = [
    'All',
    'CHASE BANK (0008678900)',
    'J.P. MORGAN SECURITIES LLC (0006868700)'
  ];

  const counterpartyOptions = [
    'All',
    'CHASE BANK (0008678900)',
    'J.P. MORGAN SECURITIES LLC (0006868700)'
  ];

  const productOptions = ['DERIVATIVES', 'EQUITIES', 'FIXED INCOME', 'COMMODITIES'];

  const handleDropdownChange = (field, value) => {
    setFormData(prev => ({ ...prev, [field]: value }));
  };

  const handleInputChange = (field, value) => {
    setFormData(prev => ({ ...prev, [field]: value }));
  };

 const constructPayload = () => {
  // Build the filter string to match the exact format from the image
  const filterParts = [];
  
  // Region filter
  filterParts.push(`{\"property\":\"region\",\"operator\":\"EQUALS\",\"value\":\"${formData.region}\"}`);
  
  // CSR filter (only add if not "All")
  if (formData.csr !== "All") {
    filterParts.push(`{\"property\":\"csr\",\"operator\":\"EQUALS\",\"value\":\"${formData.csr}\"}`);
  }
  
  // Legal Entity filter (only add if not "All")
  if (formData.legalEntity !== "All") {
    filterParts.push(`{\"property\":\"legalEntity\",\"operator\":\"EQUALS\",\"value\":\"${formData.legalEntity}\"}`);
  }
  
  // Counterparty filter (only add if not "All")
  if (formData.counterparty !== "All") {
    filterParts.push(`{\"property\":\"counterparty\",\"operator\":\"EQUALS\",\"value\":\"${formData.counterparty}\"}`);
  }
  
  // COB From date filter
  filterParts.push(`{\"property\":\"cobFrom\",\"operator\":\"GREATER_THAN_EQUALS\",\"value\":\"${formData.cobFrom}\"}`);
  
  // COB To date filter  
  filterParts.push(`{\"property\":\"cobTo\",\"operator\":\"LESS_THAN_EQUALS\",\"value\":\"${formData.cobTo}\"}`);
  
  // Product filter
  filterParts.push(`{\"property\":\"product\",\"operator\":\"EQUALS\",\"value\":\"${formData.product}\"}`);
  
  const filterString = `[${filterParts.join(',')}]`;
  
  const payload = {
    filter: filterString
  };
  
  return payload;
};

  const handleApplyFilters = async () => {
    setIsLoading(true);
    const payload = constructPayload();
    console.log('Request Payload:', JSON.stringify(payload, null, 2));
    
    try {
      const response = await fetch('http://localhost:3443/classic/test/resources/data/calc/margin/systemStatementStatusDataJoinApiJson_gtc=1793186534629', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify(payload)
      });
      
      if (response.ok) {
        const result = await response.json();
        console.log('Search results:', result);
        setResults(`Search completed successfully. Found ${result.data?.length || 0} results.`);
      } else {
        console.error('Search failed:', response.statusText);
        setResults(`Search failed: ${response.statusText}`);
      }
    } catch (error) {
      console.error('Network error:', error);
      setResults(`Network error: ${error.message}`);
    } finally {
      setIsLoading(false);
    }
  };

  const handleReset = () => {
    setFormData({
      region: 'AMERICA',
      csr: 'All',
      legalEntity: 'All',
      counterparty: 'All',
      cobFrom: '10-Oct-2022',
      cobTo: '11-Oct-2022',
      product: 'DERIVATIVES'
    });
    setResults(null);
  };

  return (
    <SaltProvider mode="light" theme="modern">
      <div style={{ minHeight: '100vh', backgroundColor: 'var(--salt-color-gray-10)', padding: '24px' }}>
        <div style={{ maxWidth: '1400px', margin: '0 auto' }}>
          
          {/* Header */}
          <Card style={{ marginBottom: '24px', padding: '24px' }}>
            <FlexLayout align="center" gap={2}>
            
              <Text styleAs="h1">Financial Data Filters</Text>
            </FlexLayout>
          </Card>

          {/* Filter Form */}
          <Card style={{ padding: '32px' }}>
            <StackLayout gap={4}>
              
              {/* First Row of Filters */}
              <div style={{ display: 'grid', gridTemplateColumns: 'repeat(auto-fit, minmax(200px, 1fr))', gap: '16px' }}>
                <FormField>
                  <FormFieldLabel>Region</FormFieldLabel>
                  <Dropdown
                    value={formData.region}
                    onSelectionChange={(_, value) => handleDropdownChange('region', value)}
                  >
                    {regionOptions.map((option) => (
                      <Option key={option} value={option}>
                        {option}
                      </Option>
                    ))}
                  </Dropdown>
                </FormField>

                <FormField>
                  <FormFieldLabel>CSR</FormFieldLabel>
                  <Dropdown
                    value={formData.csr}
                    onSelectionChange={(_, value) => handleDropdownChange('csr', value)}
                  >
                    {csrOptions.map((option) => (
                      <Option key={option} value={option}>
                        {option}
                      </Option>
                    ))}
                  </Dropdown>
                </FormField>

                <FormField>
                  <FormFieldLabel>Legal Entity</FormFieldLabel>
                  <Dropdown
                    value={formData.legalEntity}
                    onSelectionChange={(_, value) => handleDropdownChange('legalEntity', value)}
                  >
                    {legalEntityOptions.map((option) => (
                      <Option key={option} value={option}>
                        {option}
                      </Option>
                    ))}
                  </Dropdown>
                </FormField>

                <FormField>
                  <FormFieldLabel>Counterparty</FormFieldLabel>
                  <Dropdown
                    value={formData.counterparty}
                    onSelectionChange={(_, value) => handleDropdownChange('counterparty', value)}
                  >
                    {counterpartyOptions.map((option) => (
                      <Option key={option} value={option}>
                        {option}
                      </Option>
                    ))}
                  </Dropdown>
                </FormField>
              </div>

              {/* Second Row - Date Range and Product */}
              <div style={{ display: 'grid', gridTemplateColumns: 'repeat(auto-fit, minmax(150px, 1fr))', gap: '16px', alignItems: 'end' }}>
                <FormField>
                  <FormFieldLabel>COB From</FormFieldLabel>
                  <Input
                    value={formData.cobFrom}
                    onChange={(e) => handleInputChange('cobFrom', e.target.value)}
                    placeholder="DD-MMM-YYYY"
                    endAdornment={<CalendarIcon />}
                  />
                </FormField>

                <FormField>
                  <FormFieldLabel>COB To</FormFieldLabel>
                  <Input
                    value={formData.cobTo}
                    onChange={(e) => handleInputChange('cobTo', e.target.value)}
                    placeholder="DD-MMM-YYYY"
                    endAdornment={<CalendarIcon />}
                  />
                </FormField>

                <FormField>
                  <FormFieldLabel>Product</FormFieldLabel>
                  <Dropdown
                    value={formData.product}
                    onSelectionChange={(_, value) => handleDropdownChange('product', value)}
                  >
                    {productOptions.map((option) => (
                      <Option key={option} value={option}>
                        {option}
                      </Option>
                    ))}
                  </Dropdown>
                </FormField>
              </div>

              {/* Action Buttons */}
              <FlexLayout gap={2}>
                <Button 
                  variant="cta" 
                  onClick={handleApplyFilters}
                  disabled={isLoading}
                >
                  {isLoading ? 'Applying...' : 'Apply Filters'}
                </Button>
                <Button 
                  variant="secondary" 
                  onClick={handleReset}
                  disabled={isLoading}
                >
                  Reset
                </Button>
              </FlexLayout>

            </StackLayout>
          </Card>

          {/* Results Banner */}
          {results && (
            <Banner 
              status={results.includes('error') || results.includes('failed') ? 'error' : 'success'}
              style={{ marginTop: '24px' }}
            >
              {results}
            </Banner>
          )}

          {/* Current Selection Summary */}
          <Card style={{ marginTop: '24px', padding: '20px' }}>
            <Text styleAs="h3" style={{ marginBottom: '12px' }}>Current Selection Summary</Text>
            <FlexLayout direction="column" gap={1}>
              <Text><strong>Region:</strong> {formData.region}</Text>
              <Text><strong>CSR:</strong> {formData.csr}</Text>
              <Text><strong>Legal Entity:</strong> {formData.legalEntity}</Text>
              <Text><strong>Counterparty:</strong> {formData.counterparty}</Text>
              <Text><strong>COB Period:</strong> {formData.cobFrom} to {formData.cobTo}</Text>
              <Text><strong>Product:</strong> {formData.product}</Text>
            </FlexLayout>
          </Card>

        </div>
      </div>
    </SaltProvider>
  );
};

function App() {
  return (
    <div className="App">
      <FinancialFilterForm />
    </div>
  );
}

export default App;
