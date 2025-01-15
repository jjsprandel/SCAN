import React, { useState } from "react";
import { Container, Row, Col, Form, Table, Card } from "react-bootstrap";

function UserManagement() {
  // Sample data for the table
  const data = [
    {
      userId: "1",
      firstName: "John",
      lastName: "Doe",
      checkinStatus: "Check-In",
      location: "Entrance",
      totalOccupancyTime: "2 hours",
      averageStayDuration: "1.2 hours",
    },
    {
      userId: "2",
      firstName: "Jane",
      lastName: "Smith",
      checkinStatus: "Check-Out",
      location: "Exit",
      totalOccupancyTime: "3 hours",
      averageStayDuration: "1.5 hours",
    },
    {
      userId: "3",
      firstName: "Mark",
      lastName: "Lee",
      checkinStatus: "Check-In",
      location: "Entrance",
      totalOccupancyTime: "1 hour",
      averageStayDuration: "1 hour",
    },
  ];

  // State to manage the selected user's profile data
  const [profileData, setProfileData] = useState(data[0]);

  // Function to handle row click
  const handleRowClick = (user) => {
    setProfileData(user);
  };

  return (
    <Container fluid className="d-flex flex-column flex-grow-1 overflow-auto">
      {/* Row for the search bar */}
      <Row className="mt-4">
        <Col md={3}>
          <Form>
            <Form.Group controlId="search">
              <Form.Control type="text" placeholder="Search..." />
            </Form.Group>
          </Form>
        </Col>
      </Row>

      {/* Row for the table and profile card */}
      <Row className="flex-grow-1 mt-4">
        {/* Data table */}
        <Col md={6}>
          <Table striped bordered hover responsive>
            <thead>
              <tr>
                <th>User ID</th>
                <th>First Name</th>
                <th>Last Name</th>
                <th>Check-In Status</th>
              </tr>
            </thead>
            <tbody>
              {data.map((item, index) => (
                <tr
                  key={index}
                  onClick={() => handleRowClick(item)}
                  style={{ cursor: "pointer" }}
                >
                  <td>{item.userId}</td>
                  <td>{item.firstName}</td>
                  <td>{item.lastName}</td>
                  <td>{item.checkinStatus}</td>
                </tr>
              ))}
            </tbody>
          </Table>
        </Col>
        {/* Profile Card */}
        <Col md={6}>
          <Card className="mb-3 flex-grow-1">
            <Card.Header>Profile</Card.Header>
            <Card.Body className="d-flex flex-column justify-content-center align-items-center">
              <Row className="w-100 d-flex justify-content-center align-items-center">
                <Col
                  md={2}
                  className="d-flex justify-content-center align-items-center"
                >
                  <img
                    src="/imgs/user.png"
                    alt="Description"
                    className="img-fluid"
                  />
                </Col>
              </Row>
              <Row className="w-100 d-flex justify-content-center align-items-center">
                <Col
                  md={6}
                  className="d-flex justify-content-left align-items-left"
                >
                  <p>First Name:</p>
                </Col>
                <Col
                  md={6}
                  className="d-flex justify-content-left align-items-left"
                >
                  <p>{profileData.firstName}</p>
                </Col>
              </Row>
              <Row className="w-100 d-flex justify-content-center align-items-center">
                <Col
                  md={6}
                  className="d-flex justify-content-left align-items-left"
                >
                  <p>Last Name:</p>
                </Col>
                <Col
                  md={6}
                  className="d-flex justify-content-left align-items-left"
                >
                  <p>{profileData.lastName}</p>
                </Col>
              </Row>
              <Row className="w-100 d-flex justify-content-center align-items-center">
                <Col
                  md={6}
                  className="d-flex justify-content-left align-items-left"
                >
                  <p>Location:</p>
                </Col>
                <Col
                  md={6}
                  className="d-flex justify-content-left align-items-left"
                >
                  <p>{profileData.location}</p>
                </Col>
              </Row>
              <Row className="w-100 d-flex justify-content-center align-items-center">
                <Col
                  md={6}
                  className="d-flex justify-content-left align-items-left"
                >
                  <p>Total Occupancy Time:</p>
                </Col>
                <Col
                  md={6}
                  className="d-flex justify-content-left align-items-left"
                >
                  <p>{profileData.totalOccupancyTime}</p>
                </Col>
              </Row>
              <Row className="w-100 d-flex justify-content-center align-items-center">
                <Col
                  md={6}
                  className="d-flex justify-content-left align-items-left"
                >
                  <p>Average Stay Duration:</p>
                </Col>
                <Col
                  md={6}
                  className="d-flex justify-content-left align-items-left"
                >
                  <p>{profileData.averageStayDuration}</p>
                </Col>
              </Row>
            </Card.Body>
          </Card>
        </Col>
      </Row>
    </Container>
  );
}

export default UserManagement;