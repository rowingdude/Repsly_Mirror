CREATE VIEW sales.purchase_order_items_calculated AS
SELECT
    poi.*,
    (poi.quantity * poi.unit_price) AS amount,
    (poi.quantity * poi.unit_price * (poi.discount_percent / 100)) AS discount_amount,
    (poi.quantity * poi.unit_price * (1 - poi.discount_percent / 100) * (poi.tax_percent / 100)) AS tax_amount,
    (poi.quantity * poi.unit_price * (1 - poi.discount_percent / 100) * (1 + poi.tax_percent / 100)) AS total_amount
FROM
    sales.purchase_order_items poi;