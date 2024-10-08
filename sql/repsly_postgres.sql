CREATE SCHEMA IF NOT EXISTS core;
CREATE SCHEMA IF NOT EXISTS sales;
CREATE SCHEMA IF NOT EXISTS inventory;
CREATE SCHEMA IF NOT EXISTS field_ops;
CREATE SCHEMA IF NOT EXISTS user_mgmt;
CREATE SCHEMA IF NOT EXISTS meta;
CREATE SCHEMA IF NOT EXISTS geo;
CREATE SCHEMA IF NOT EXISTS geography;


-- Meta tables
CREATE TABLE meta.notes (
    note_id SERIAL PRIMARY KEY,
    note_text TEXT
);

CREATE TABLE meta.time (
    time_id SERIAL PRIMARY KEY,
    timestamp TIMESTAMP
);

CREATE TABLE meta.date (
    date_id SERIAL PRIMARY KEY,
    date DATE
);

CREATE TABLE meta.mileage (
    mileage_id SERIAL PRIMARY KEY,
    mileage INTEGER
);

CREATE TABLE meta.custom_fields (
    field_id SERIAL PRIMARY KEY,
    entity_type VARCHAR(50),
    field_name VARCHAR(255),
    field_type VARCHAR(50)
);

CREATE TABLE meta.custom_field_values (
    value_id SERIAL PRIMARY KEY,
    field_id INTEGER REFERENCES meta.custom_fields(field_id),
    entity_id INTEGER,
    value TEXT
);

CREATE TABLE meta.attributes (
    attribute_id SERIAL PRIMARY KEY,
    title VARCHAR(255),
    attribute_type VARCHAR(50)
);

CREATE TABLE meta.attribute_values (
    value_id SERIAL PRIMARY KEY,
    attribute_id INTEGER REFERENCES meta.attributes(attribute_id),
    entity_type VARCHAR(50),
    entity_id INTEGER,
    value TEXT
);

-- Geography tables
CREATE TABLE geography.countries (
    country_id SERIAL PRIMARY KEY,
    name VARCHAR(255) UNIQUE,
    code VARCHAR(20) UNIQUE
);

CREATE TABLE geography.states (
    state_id SERIAL PRIMARY KEY,
    name VARCHAR(255),
    code VARCHAR(20),
    country_id INTEGER REFERENCES geography.countries(country_id),
    UNIQUE(name, country_id),
    UNIQUE(code, country_id)
);

CREATE TABLE geography.cities (
    city_id SERIAL PRIMARY KEY,
    name VARCHAR(255),
    state_id INTEGER REFERENCES geography.states(state_id),
    UNIQUE(name, state_id)
);

CREATE TABLE geography.zip_codes (
    zip_code_id SERIAL PRIMARY KEY,
    code VARCHAR(20),
    ext VARCHAR(20),
    city_id INTEGER REFERENCES geography.cities(city_id),
    UNIQUE(code, ext, city_id)
);

-- Geo tables
CREATE TABLE geo.lat (
    lat_id SERIAL PRIMARY KEY,
    latitude DECIMAL(9,6)
);

CREATE TABLE geo.long (
    long_id SERIAL PRIMARY KEY,
    longitude DECIMAL(9,6)
);





CREATE TABLE core.addresses (
    address_id SERIAL PRIMARY KEY,
    street_address VARCHAR(255),
    zip_code_id INTEGER REFERENCES geography.zip_codes(zip_code_id),
    lat_id INTEGER REFERENCES geo.lat(lat_id),
    long_id INTEGER REFERENCES geo.long(long_id)
);

CREATE TABLE core.contact_info (
    contact_id SERIAL PRIMARY KEY,
    phone VARCHAR(128),
    mobile VARCHAR(128),
    website VARCHAR(255)
);

CREATE TABLE core.territories (
    territory_id SERIAL PRIMARY KEY,
    name VARCHAR(80),
    parent_territory_id INTEGER REFERENCES core.territories(territory_id)
);

CREATE TABLE core.tags (
    tag_id SERIAL PRIMARY KEY,
    name VARCHAR(255) UNIQUE
);

CREATE TABLE core.emails (
    email_id SERIAL PRIMARY KEY,
    email_address VARCHAR(255) UNIQUE
);

CREATE TABLE core.roles (
    role_id SERIAL PRIMARY KEY,
    role_name VARCHAR(80) UNIQUE
);

CREATE TABLE core.names (
    name_id SERIAL PRIMARY KEY,
    full_name VARCHAR(255)
);


CREATE TABLE field_ops.rep_code (
    rep_code_id SERIAL PRIMARY KEY,
    rep_code VARCHAR(20) UNIQUE
);

CREATE TABLE field_ops.representatives (
    rep_id SERIAL PRIMARY KEY,
    active BOOLEAN,
    rep_code_id INTEGER REFERENCES field_ops.rep_code(rep_code_id),
    note_id INTEGER REFERENCES meta.notes(note_id),
    address_id INTEGER REFERENCES core.addresses(address_id),
    contact_id INTEGER REFERENCES core.contact_info(contact_id),
    email_id INTEGER REFERENCES core.emails(email_id),
    name_id INTEGER REFERENCES core.names(name_id)
);

CREATE TABLE field_ops.rep_territories (
    rep_id INTEGER REFERENCES field_ops.representatives(rep_id),
    territory_id INTEGER REFERENCES core.territories(territory_id),
    PRIMARY KEY (rep_id, territory_id)
);

CREATE TABLE field_ops.visits (
    visit_id SERIAL PRIMARY KEY,
    client_id INTEGER,  -- Will be referenced later
    rep_id INTEGER REFERENCES field_ops.representatives(rep_id),
    time_start_id INTEGER REFERENCES meta.time(time_id),
    time_end_id INTEGER REFERENCES meta.time(time_id),
    explicit_check_in BOOLEAN,
    lat_start_id INTEGER REFERENCES geo.lat(lat_id),
    long_start_id INTEGER REFERENCES geo.long(long_id),
    lat_end_id INTEGER REFERENCES geo.lat(lat_id),
    long_end_id INTEGER REFERENCES geo.long(long_id),
    precision_start INTEGER,
    precision_end INTEGER,
    visit_status_by_schedule INTEGER,
    visit_ended BOOLEAN,
    duration_minutes INTEGER 
);

CREATE TABLE field_ops.daily_working_time (
    dwt_id SERIAL PRIMARY KEY,
    rep_id INTEGER REFERENCES field_ops.representatives(rep_id),
    date_id INTEGER REFERENCES meta.date(date_id),
    time_start_id INTEGER REFERENCES meta.time(time_id),
    time_end_id INTEGER REFERENCES meta.time(time_id),
    mileage_start_id INTEGER REFERENCES meta.mileage(mileage_id),
    mileage_end_id INTEGER REFERENCES meta.mileage(mileage_id),
    lat_start_id INTEGER REFERENCES geo.lat(lat_id),
    long_start_id INTEGER REFERENCES geo.long(long_id),
    lat_end_id INTEGER REFERENCES geo.lat(lat_id),
    long_end_id INTEGER REFERENCES geo.long(long_id),
    note_id INTEGER REFERENCES meta.notes(note_id),
    no_of_visits INTEGER,
    min_of_visits INTEGER REFERENCES meta.time(time_id),
    max_of_visits INTEGER REFERENCES meta.time(time_id),
    min_max_visits_time INTEGER,
    time_at_client INTEGER,
    time_at_travel INTEGER,
    duration_minutes INTEGER
);


CREATE TABLE field_ops.working_day_tags (
    dwt_id INTEGER REFERENCES field_ops.daily_working_time(dwt_id),
    tag_id INTEGER REFERENCES core.tags(tag_id),
    PRIMARY KEY (dwt_id, tag_id)
);

CREATE TABLE field_ops.visit_schedules (
    schedule_id SERIAL PRIMARY KEY,
    rep_id INTEGER REFERENCES field_ops.representatives(rep_id),
    client_id INTEGER,  -- Will be referenced later
    schedule_time_id INTEGER REFERENCES meta.time(time_id),
    visit_note_id INTEGER REFERENCES meta.notes(note_id),
    due_date_id INTEGER REFERENCES meta.date(date_id)
);

CREATE TABLE field_ops.retail_audits (
    audit_id SERIAL PRIMARY KEY,
    name VARCHAR(50),
    cancelled BOOLEAN,
    client_id INTEGER,  -- Will be referenced later
    time_id INTEGER REFERENCES meta.time(time_id),
    rep_id INTEGER REFERENCES field_ops.representatives(rep_id),
    visit_id INTEGER REFERENCES field_ops.visits(visit_id),
    note_id INTEGER REFERENCES meta.notes(note_id)
);

-- Sales tables
CREATE TABLE sales.clients (
    client_id SERIAL PRIMARY KEY,
    code VARCHAR(50) UNIQUE,
    active BOOLEAN,
    address_id INTEGER REFERENCES core.addresses(address_id),
    contact_id INTEGER REFERENCES core.contact_info(contact_id),
    territory_id INTEGER REFERENCES core.territories(territory_id),
    rep_id INTEGER REFERENCES field_ops.representatives(rep_id),
    account_code TEXT,
    status VARCHAR(255),
    contact_name_id INTEGER REFERENCES core.names(name_id),
    contact_title_id INTEGER REFERENCES core.names(name_id),
    name_id INTEGER REFERENCES core.names(name_id)
);

CREATE TABLE field_ops.photos (
    photo_id SERIAL PRIMARY KEY,
    visit_id INTEGER REFERENCES field_ops.visits(visit_id),
    client_id INTEGER REFERENCES sales.clients(client_id),
    rep_id INTEGER REFERENCES field_ops.representatives(rep_id),
    note TEXT,
    date_time TIMESTAMP,
    photo_url TEXT
);

ALTER TABLE field_ops.visits ADD CONSTRAINT fk_visits_client FOREIGN KEY (client_id) REFERENCES sales.clients(client_id);
ALTER TABLE field_ops.photos ADD CONSTRAINT fk_photos_client FOREIGN KEY (client_id) REFERENCES sales.clients(client_id);
ALTER TABLE field_ops.visit_schedules ADD CONSTRAINT fk_visit_schedules_client FOREIGN KEY (client_id) REFERENCES sales.clients(client_id);
ALTER TABLE field_ops.retail_audits ADD CONSTRAINT fk_retail_audits_client FOREIGN KEY (client_id) REFERENCES sales.clients(client_id);


CREATE TABLE sales.client_tags (
    client_id INTEGER REFERENCES sales.clients(client_id),
    tag_id INTEGER REFERENCES core.tags(tag_id),
    PRIMARY KEY (client_id, tag_id)
);

-- Document Types
CREATE TABLE sales.document_types (
    document_type_id SERIAL PRIMARY KEY,
    name VARCHAR(100) UNIQUE,
    attribute_caption TEXT
);

-- Document Statuses
CREATE TABLE sales.document_statuses (
    document_status_id SERIAL PRIMARY KEY,
    name VARCHAR(100) UNIQUE
);

-- Package Types
CREATE TABLE inventory.package_types (
    package_type_id SERIAL PRIMARY KEY,
    code VARCHAR(45) UNIQUE,
    name VARCHAR(40),
    conversion INTEGER
);


CREATE TABLE inventory.product_groups (
    group_id SERIAL PRIMARY KEY,
    code VARCHAR(20) UNIQUE,
    name VARCHAR(80)
);


-- Document Item Attributes
CREATE TABLE sales.document_item_attributes (
    attribute_id SERIAL PRIMARY KEY,
    name VARCHAR(100) UNIQUE
);

CREATE TABLE inventory.products (
    product_id SERIAL PRIMARY KEY,
    code VARCHAR(20) UNIQUE,
    name VARCHAR(80),
    group_id INTEGER REFERENCES inventory.product_groups(group_id),
    active BOOLEAN,
    unit_price DECIMAL(18,4),
    ean VARCHAR(20),
    note_id INTEGER REFERENCES meta.notes(note_id),
    image_url TEXT,
    master_product_id INTEGER REFERENCES inventory.products(product_id)
);

CREATE TABLE inventory.product_tags (
    product_id INTEGER REFERENCES inventory.products(product_id),
    tag_id INTEGER REFERENCES core.tags(tag_id),
    PRIMARY KEY (product_id, tag_id)
);


CREATE TABLE sales.purchase_orders (
    order_id SERIAL PRIMARY KEY,
    transaction_type VARCHAR(50),
    document_type_id INTEGER REFERENCES sales.document_types(document_type_id),
    document_status_id INTEGER REFERENCES sales.document_statuses(document_status_id),
    time_id INTEGER REFERENCES meta.time(time_id),
    document_no VARCHAR(50) UNIQUE,
    client_id INTEGER REFERENCES sales.clients(client_id),
    document_date_id INTEGER REFERENCES meta.date(date_id),
    due_date_id INTEGER REFERENCES meta.date(date_id),
    rep_id INTEGER REFERENCES field_ops.representatives(rep_id),
    signature_url TEXT,
    note_id INTEGER REFERENCES meta.notes(note_id),
    taxable BOOLEAN,
    visit_id INTEGER REFERENCES field_ops.visits(visit_id),
    original_document_number TEXT
);

CREATE TABLE sales.document_item_attributes (
    attribute_id SERIAL PRIMARY KEY,
    name VARCHAR(100) UNIQUE
);

CREATE TABLE sales.purchase_order_items (
    item_id SERIAL PRIMARY KEY,
    order_id INTEGER REFERENCES sales.purchase_orders(order_id),
    line_no INTEGER,
    product_id INTEGER REFERENCES inventory.products(product_id),
    package_type_id INTEGER REFERENCES inventory.package_types(package_type_id),
    quantity INTEGER,
    unit_price DECIMAL(18,4),
    discount_percent DECIMAL(5,2),
    tax_percent DECIMAL(5,2),
    note_id INTEGER REFERENCES meta.notes(note_id),
    document_item_attribute_id INTEGER REFERENCES sales.document_item_attributes(attribute_id)
);

ALTER TABLE sales.purchase_order_items ADD CONSTRAINT fk_purchase_order_items_product FOREIGN KEY (product_id) REFERENCES inventory.products(product_id);

CREATE TABLE inventory.pricelists (
    pricelist_id SERIAL PRIMARY KEY,
    name VARCHAR(255),
    is_default BOOLEAN,
    active BOOLEAN,
    use_prices BOOLEAN
);

CREATE TABLE inventory.pricelist_items (
    item_id SERIAL PRIMARY KEY,
    pricelist_id INTEGER REFERENCES inventory.pricelists(pricelist_id),
    product_id INTEGER REFERENCES inventory.products(product_id),
    price DECIMAL(18,4),
    active BOOLEAN,
    client_id INTEGER REFERENCES sales.clients(client_id),
    manufacture_id VARCHAR(255),
    date_available_from_id INTEGER REFERENCES meta.date(date_id),
    date_available_to_id INTEGER REFERENCES meta.date(date_id),
    min_quantity INTEGER,
    max_quantity INTEGER
);

CREATE TABLE field_ops.retail_audit_items (
    item_id SERIAL PRIMARY KEY,
    audit_id INTEGER REFERENCES field_ops.retail_audits(audit_id),
    product_id INTEGER REFERENCES inventory.products(product_id),
    present BOOLEAN,
    price DECIMAL(18,4),
    promotion BOOLEAN,
    shelf_share DECIMAL(18,4),
    shelf_share_percent DECIMAL(18,4),
    sold_out BOOLEAN,
    stock INTEGER
);


CREATE TABLE user_mgmt.users (
    user_id SERIAL PRIMARY KEY,
    code VARCHAR(20) UNIQUE,
    active BOOLEAN,
    note_id INTEGER REFERENCES meta.notes(note_id),
    phone VARCHAR(128),
    send_email_enabled BOOLEAN,
    address_id INTEGER REFERENCES core.addresses(address_id),
    email_id INTEGER REFERENCES core.emails(email_id),
    role_id INTEGER REFERENCES core.roles(role_id),
    name_id INTEGER REFERENCES core.names(name_id)
);

CREATE TABLE user_mgmt.user_territories (
    user_id INTEGER REFERENCES user_mgmt.users(user_id),
    territory_id INTEGER REFERENCES core.territories(territory_id),
    PRIMARY KEY (user_id, territory_id)
);

CREATE TABLE user_mgmt.user_permissions (
    user_id INTEGER REFERENCES user_mgmt.users(user_id),
    permission VARCHAR(255),
    PRIMARY KEY (user_id, permission)
);


CREATE INDEX idx_visits_client_id ON field_ops.visits(client_id);
CREATE INDEX idx_visits_rep_id ON field_ops.visits(rep_id);
CREATE INDEX idx_visits_time_start ON field_ops.visits(time_start_id);
CREATE INDEX idx_visits_time_end ON field_ops.visits(time_end_id);

CREATE INDEX idx_daily_working_time_rep_id ON field_ops.daily_working_time(rep_id);
CREATE INDEX idx_daily_working_time_date ON field_ops.daily_working_time(date_id);

CREATE INDEX idx_purchase_orders_client_id ON sales.purchase_orders(client_id);
CREATE INDEX idx_purchase_orders_rep_id ON sales.purchase_orders(rep_id);
CREATE INDEX idx_purchase_orders_time ON sales.purchase_orders(time_id);

CREATE INDEX idx_products_group_id ON inventory.products(group_id);
CREATE INDEX idx_pricelist_items_product_id ON inventory.pricelist_items(product_id);
CREATE INDEX idx_pricelist_items_client_id ON inventory.pricelist_items(client_id);
