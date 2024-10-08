CREATE VIEW vw_client_details AS
SELECT 
    c.client_id,
    c.code,
    n.full_name AS client_name,
    cn.full_name AS contact_name,
    ct.full_name AS contact_title,
    a.street_address,
    z.code AS zip_code,
    z.ext AS zip_ext,
    ci.name AS city,
    s.name AS state,
    co.name AS country,
    t.name AS territory,
    r.rep_code AS rep_code,
    rn.full_name AS rep_name
FROM 
    sales.clients c
    LEFT JOIN core.names n ON c.name_id = n.name_id
    LEFT JOIN core.names cn ON c.contact_name_id = cn.name_id
    LEFT JOIN core.names ct ON c.contact_title_id = ct.name_id
    LEFT JOIN core.addresses a ON c.address_id = a.address_id
    LEFT JOIN geography.zip_codes z ON a.zip_code_id = z.zip_code_id
    LEFT JOIN geography.cities ci ON z.city_id = ci.city_id
    LEFT JOIN geography.states s ON ci.state_id = s.state_id
    LEFT JOIN geography.countries co ON s.country_id = co.country_id
    LEFT JOIN core.territories t ON c.territory_id = t.territory_id
    LEFT JOIN field_ops.representatives rep ON c.rep_id = rep.rep_id
    LEFT JOIN field_ops.rep_code r ON rep.rep_code_id = r.rep_code_id
    LEFT JOIN core.names rn ON rep.name_id = rn.name_id;