
def XMLDict(element):
    """
    Simple conversion of an ElementTree XML element to dict.
    Not a general-purpose XML --> JSON converter,
    but guaranteed to support DAQ config file schemas.
    """
    # For a simple text-only element, just return the text
    text = ""
    if element.text is not None:
        text = (element.text).strip()
    if len(element.getchildren()) == 0 and len(element.attrib) == 0:
        return text
    map = {}
    # Recursively parse data from child elements
    for child in element.getchildren():
        value = XMLDict(child)
        if child.tag not in map:
            map[child.tag] = value
        else:
            if type(map[child.tag]) is not list:
                map[child.tag] = [map[child.tag]]
            (map[child.tag]).append(value)
    # Prepend '@' to attribute names since these can be
    # the same as child element names
    for (key, value) in element.attrib.items():
        map['@%s' % key] = value
    # If we have text, we need to parse it.
    if len(text) > 0:
        map['@@text'] = text
    return map
