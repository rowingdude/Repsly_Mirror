CREATE VIEW vw_visit_details AS
SELECT 
    v.visit_id,
    c.code AS client_code,
    n.full_name AS client_name,
    r.rep_code,
    rn.full_name AS rep_name,
    ts.timestamp AS start_time,
    te.timestamp AS end_time,
    v.duration_minutes,
    v.explicit_check_in,
    v.visit_status_by_schedule,
    v.visit_ended
FROM 
    field_ops.visits v
    JOIN sales.clients c ON v.client_id = c.client_id
    JOIN core.names n ON c.name_id = n.name_id
    JOIN field_ops.representatives rep ON v.rep_id = rep.rep_id
    JOIN field_ops.rep_code r ON rep.rep_code_id = r.rep_code_id
    JOIN core.names rn ON rep.name_id = rn.name_id
    JOIN meta.time ts ON v.time_start_id = ts.time_id
    JOIN meta.time te ON v.time_end_id = te.time_id;

CREATE VIEW vw_purchase_order_summary AS
SELECT 
    po.order_id,
    po.document_no,
    c.code AS client_code,
    n.full_name AS client_name,
    r.rep_code,
    rn.full_name AS rep_name,
    t.timestamp AS order_time,
    d.date AS document_date,
    dd.date AS due_date,
    po.transaction_type,
    po.document_status,
    COUNT(poi.item_id) AS item_count,
    SUM(poi.total_amount) AS total_amount
FROM 
    sales.purchase_orders po
    JOIN sales.clients c ON po.client_id = c.client_id
    JOIN core.names n ON c.name_id = n.name_id
    JOIN field_ops.representatives rep ON po.rep_id = rep.rep_id
    JOIN field_ops.rep_code r ON rep.rep_code_id = r.rep_code_id
    JOIN core.names rn ON rep.name_id = rn.name_id
    JOIN meta.time t ON po.time_id = t.time_id
    JOIN meta.date d ON po.document_date_id = d.date_id
    JOIN meta.date dd ON po.due_date_id = dd.date_id
    LEFT JOIN sales.purchase_order_items poi ON po.order_id = poi.order_id
GROUP BY 
    po.order_id, po.document_no, c.code, n.full_name, r.rep_code, rn.full_name, 
    t.timestamp, d.date, dd.date, po.transaction_type, po.document_status;
