CREATE OR REPLACE FUNCTION calculate_visit_duration(start_time_id INTEGER, end_time_id INTEGER)
RETURNS INTEGER AS $$
DECLARE
    duration INTEGER;
BEGIN
    SELECT EXTRACT(EPOCH FROM (end_time.timestamp - start_time.timestamp)) / 60
    INTO duration
    FROM meta.time start_time
    JOIN meta.time end_time ON end_time.time_id = end_time_id
    WHERE start_time.time_id = start_time_id;
    
    RETURN duration;
END;
$$ LANGUAGE plpgsql;