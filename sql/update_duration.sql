CREATE OR REPLACE FUNCTION update_visit_duration()
RETURNS TRIGGER AS $$
BEGIN
    NEW.duration_minutes = calculate_visit_duration(NEW.time_start_id, NEW.time_end_id);
    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER tr_update_visit_duration
BEFORE INSERT OR UPDATE ON field_ops.visits
FOR EACH ROW EXECUTE FUNCTION update_visit_duration();