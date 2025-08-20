const constructPayload = () => {
  const filters = [
    { property: "region", operator: "EQUALS", value: formData.region },
    { property: "cobFrom", operator: "GREATER_THAN_EQUALS", value: formData.cobFrom },
    { property: "cobTo", operator: "LESS_THAN_EQUALS", value: formData.cobTo },
    { property: "product", operator: "EQUALS", value: formData.product }
  ];

  // Add conditional filters only if not "All"
  ["csr", "legalEntity", "counterparty"].forEach((key) => {
    if (formData[key] !== "All") {
      filters.push({ property: key, operator: "EQUALS", value: formData[key] });
    }
  });

  return {
    filter: JSON.stringify(filters)
  };
};
